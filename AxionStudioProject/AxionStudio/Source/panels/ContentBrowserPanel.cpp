#include "ContentBrowserPanel.h"

#include "AxionEngine/Vendor/imgui/imgui.h"

#include "AxionEngine/Source/core/AssetManager.h"
#include "AxionEngine/Source/core/PlatformUtils.h"

namespace Axion {

	constexpr float iconSize = 30.0f;
	static const std::filesystem::path s_assetPath = "AxionStudio/Assets"; // TODO: make this part of a project config

	ContentBrowserPanel::ContentBrowserPanel(const std::string& name) : Panel(name) {
	}

	ContentBrowserPanel::~ContentBrowserPanel() {
		shutdown();
	}

	void ContentBrowserPanel::setup() {
		std::error_code ec;
		m_rootDirectory = std::filesystem::weakly_canonical(s_assetPath, ec);
		if (ec) m_rootDirectory = std::filesystem::absolute(s_assetPath, ec);

		m_currentDirectory = m_rootDirectory;

		// TODO: create a editor own resource handler for such icons
		m_folderIcon =		Texture2D::create("AxionStudio/Resources/contentbrowser/FolderIcon.png");
		m_fileIcon =		Texture2D::create("AxionStudio/Resources/contentbrowser/FileIcon.png");
		m_backIcon =		Texture2D::create("AxionStudio/Resources/contentbrowser/BackIcon.png");
		m_refreshIcon =		Texture2D::create("AxionStudio/Resources/contentbrowser/RefreshIcon.png");
		m_addFolderIcon =	Texture2D::create("AxionStudio/Resources/contentbrowser/AddFolderIcon.png");

		refreshDirectory();
	}

	void ContentBrowserPanel::shutdown() {
		m_folderIcon->release();
		m_fileIcon->release();
		m_backIcon->release();
		m_refreshIcon->release();
		m_addFolderIcon->release();
	}

	void ContentBrowserPanel::onGuiRender() {
		ImGui::Begin("Content Browser");

		static float padding = 8.0f;
		static float thumbnailSize = 128.0f;

		// ----- Scrolling for zoom -----
		if (ImGui::IsWindowHovered(ImGuiHoveredFlags_AllowWhenBlockedByActiveItem)) {
			ImGuiIO& io = ImGui::GetIO();
			float scrollY = io.MouseWheel;
			if (io.KeyCtrl && scrollY != 0) {
				thumbnailSize += (io.KeyShift) ? scrollY : scrollY * 5.0f;
				thumbnailSize = std::clamp(thumbnailSize, 16.0f, 512.0f);
				m_showNames = thumbnailSize >= 50;
			}
		}


		// ----- Draw toolbar -----
		drawToolbar();


		// ----- Draw ContentBrowser -----
		float cellSize = thumbnailSize + padding;
		float panelWidth = ImGui::GetContentRegionAvail().x;
		int colCount = (int)(panelWidth / cellSize);
		if (colCount < 1) { colCount = 1; }
		ImGui::Columns(colCount, 0, false);

		for (const auto& item : m_directoryEntries) {
			const auto& path = item.path;
			const std::string& filenameString = item.displayName;
			Ref<Texture2D> icon = item.isDir ? m_folderIcon : m_fileIcon;

			// -- Apply search filter --
			if (!matchesSearch(filenameString)) continue;


			// -- Draw file / folder icon --
			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
			std::string uniqueID = "##" + filenameString;
			if (ImGui::ImageButton((uniqueID + "_icon").c_str(), reinterpret_cast<ImTextureID>(icon->getHandle()), { thumbnailSize, thumbnailSize }, { 0, 1 }, { 1, 0 })) {
				if (item.isDir) {
					// -- Clicked on folder --
					m_pendingNavigate = m_currentDirectory / path.filename();
				}
				else {
					// -- Clicked on file --
				}
			}


			// -- Draw options on right click --
			if (ImGui::BeginPopupContextItem(filenameString.c_str())) {
				// -- Show in explorer button --
				if (ImGui::MenuItem("Show in Explorer")) {
					PlatformUtils::showInFileExplorer(path.string()); 
				}

				// -- Renaming --
				if (ImGui::MenuItem("Rename")) {
					m_itemBeingRenamed = path;
					std::string fn = path.filename().string();
					strncpy_s(m_itemRenameBuffer, sizeof(m_itemRenameBuffer), fn.c_str(), _TRUNCATE);
					m_startRenaming = true;
				}

				// -- Deleting --
				ImGui::Separator();
				if (ImGui::MenuItem("Delete")) {
					m_pendingDelete = path;
					m_openDeletePopup = true;
				}

				ImGui::EndPopup();
			}


			// -- Drag and drop source --
			if (ImGui::BeginDragDropSource()) {
				auto rel = path.lexically_relative(m_rootDirectory);
				const std::string itemPath = rel.string();
				ImGui::SetDragDropPayload("CONTENT_BROWSER_ITEM", itemPath.c_str(), itemPath.size() + 1);
				ImGui::Image(reinterpret_cast<ImTextureID>(icon->getHandle()), { 30.0f, 30.0f }, { 0, 1 }, { 1, 0 });
				ImGui::EndDragDropSource();
			}


			// -- Draw name / renaming field --
			ImGui::PopStyleColor();
			if (m_showNames) {
				// -- Renaming --
				if (m_itemBeingRenamed == path) {
					ImGui::SetNextItemWidth(thumbnailSize);
					if (m_startRenaming) {
						ImGui::SetKeyboardFocusHere();
						m_startRenaming = false;
					}

					// -- Draw input field --
					if (ImGui::InputText("##RenameInput", m_itemRenameBuffer, IM_ARRAYSIZE(m_itemRenameBuffer), ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_AutoSelectAll)) {
						std::string newName(m_itemRenameBuffer);
						if (!newName.empty()) {
							auto newPath = path.parent_path() / newName;

							// -- Check if name is valid --
							std::error_code ec;
							if (!std::filesystem::exists(newPath, ec) && !ec) {
								m_pendingRename = std::make_pair(path, newPath);
							}
							else if (!ec) {
								if (std::filesystem::equivalent(path, newPath, ec) && !ec) {
									// nothing to do
								}
								else {
									AX_CORE_LOG_WARN("Cannot rename: target already exists: {}", newPath.string());
								}
							}
							else {
								AX_CORE_LOG_ERROR("Filesystem check failed: {}", ec.message());
							}
						}

						resetRenaming();
					}

					// cancel renaming on clicking outside
					if (ImGui::IsItemDeactivated() && !ImGui::IsItemDeactivatedAfterEdit()) {
						resetRenaming();
					}

					// cancel renaming on escape
					if (ImGui::IsKeyPressed(ImGuiKey_Escape)) {
						resetRenaming();
					}
				}
				// -- Draw name --
				else {
					ImVec2 textSize = ImGui::CalcTextSize(filenameString.c_str());
					float textX = std::max(ImGui::GetCursorPosX(), ImGui::GetCursorPosX() + (thumbnailSize - textSize.x) * 0.5f);
					ImGui::SetCursorPosX(textX);
					ImGui::TextUnformatted(filenameString.c_str());
				}
			}

			ImGui::NextColumn();

		}
		ImGui::Columns(1);


		// ----- Execute renaming after the loop -----
		if (m_pendingRename.has_value()) {
			const auto& [oldPath, newPath] = *m_pendingRename;
			std::error_code ec;
			// -- Final safety checks --
			if (!std::filesystem::exists(newPath, ec)) {
				std::filesystem::rename(oldPath, newPath, ec);
				if (ec) AX_CORE_LOG_ERROR("Rename failed: {}", ec.message());
			}
			else {
				AX_CORE_LOG_WARN("Rename target already exists: {}", newPath.string());
			}
			m_pendingRename.reset();
			refreshDirectory();
		}


		// ----- Execute navigating to folder after the loop -----
		if (m_pendingNavigate.has_value()) {
			m_currentDirectory = *m_pendingNavigate;
			m_pendingNavigate.reset();
			refreshDirectory();
		}


		// ----- Open delete confirmation -----
		if (m_openDeletePopup) {
			ImGui::OpenPopup("Confirm Delete");
			m_openDeletePopup = false;
		}


		// ----- Draw delete confirmation -----
		if (ImGui::BeginPopupModal("Confirm Delete", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
			std::string msg = "Are you sure you want to delete:\n" + m_pendingDelete->filename().string() + "?";
			ImGui::TextWrapped("%s", msg.c_str());
			ImGui::Separator();

			// -- Yes button --
			if (ImGui::Button("Yes", ImVec2(120, 0))) {
				deletePath(*m_pendingDelete);
				m_pendingDelete.reset();
				refreshDirectory();
				ImGui::CloseCurrentPopup();
			}

			// -- No button --
			ImGui::SameLine();
			if (ImGui::Button("No", ImVec2(120, 0))) {
				m_pendingDelete.reset();
				ImGui::CloseCurrentPopup();
			}

			// -- Cancel on escape --
			if (ImGui::IsKeyPressed(ImGuiKey_Escape)) {
				m_pendingDelete.reset();
				ImGui::CloseCurrentPopup();
			}

			ImGui::EndPopup();
			
		}

		ImGui::End();
	}

	void ContentBrowserPanel::refreshDirectory() {
		std::vector<DirItem> tmp;
		tmp.reserve(64);

		std::error_code ec;
		std::filesystem::directory_iterator it(m_currentDirectory, std::filesystem::directory_options::skip_permission_denied, ec);
		if (ec) {
			m_directoryEntries.clear();
			return;
		}

		for (auto end = std::filesystem::directory_iterator(); it != end; it.increment(ec)) {
			if (ec) { ec.clear(); continue; }

			const auto p = it->path();
			bool isDir = it->is_directory(ec);
			if (ec) { ec.clear(); isDir = false; }

			DirItem di;
			di.path = p;
			di.isDir = isDir;

			auto rel = p.lexically_relative(m_rootDirectory);
			di.displayName = rel.empty() ? p.filename().string() : rel.filename().string();

			tmp.push_back(std::move(di));
		}

		m_directoryEntries.swap(tmp);
	}

	void ContentBrowserPanel::resetRenaming() {
		m_itemBeingRenamed.clear();
		m_itemRenameBuffer[0] = '\0';
		m_startRenaming = false;
	}

	bool ContentBrowserPanel::matchesSearch(const std::string& name) {
		if (m_searchBuffer[0] == '\0') return true;
		auto lower = [](std::string s) {
			std::transform(s.begin(), s.end(), s.begin(), ::tolower);
			return s;
		};
		return lower(name).find(lower(m_searchBuffer)) != std::string::npos;
	}

	void ContentBrowserPanel::deletePath(const std::filesystem::path& path) {
		std::error_code ec;
		if (std::filesystem::is_directory(path, ec) && !ec) {
			std::filesystem::remove_all(path, ec);
		}
		else {
			std::filesystem::remove(path, ec);
		}

		if (ec) { AX_CORE_LOG_ERROR("Delete failed: {}", ec.message()); }
	}

	void ContentBrowserPanel::drawToolbar() {
		// ----- Go a folder back button -----
		ImGui::BeginDisabled(m_currentDirectory == m_rootDirectory);
		if (ImGui::ImageButton("##Back_icon", reinterpret_cast<ImTextureID>(m_backIcon->getHandle()), { iconSize, iconSize })) {
			m_currentDirectory = m_currentDirectory.parent_path();
			refreshDirectory();
		}
		ImGui::EndDisabled();


		// ----- Refresh button -----
		ImGui::SameLine();
		if (ImGui::ImageButton("##Refresh_icon", reinterpret_cast<ImTextureID>(m_refreshIcon->getHandle()), { iconSize, iconSize }, { 0, 1 }, { 1, 0 })) {
			refreshDirectory();
		}


		// ----- Add folder button -----
		ImGui::SameLine();
		if (ImGui::ImageButton("##AddFolder_icon", reinterpret_cast<ImTextureID>(m_addFolderIcon->getHandle()), { iconSize, iconSize }, { 0, 1 }, { 1, 0 })) {
			std::string baseName = "New Folder";
			std::filesystem::path newFolderPath = m_currentDirectory / baseName;

			int counter = 1;
			while (std::filesystem::exists(newFolderPath)) {
				newFolderPath = m_currentDirectory / (baseName + " " + std::to_string(counter));
				++counter;
			}

			std::error_code ec;
			std::filesystem::create_directory(newFolderPath, ec);
			if (!ec) {
				// -- Allowing instant renaming of new folders --
				refreshDirectory();
				m_itemBeingRenamed = newFolderPath;
				std::string fn = newFolderPath.filename().string();
				strncpy_s(m_itemRenameBuffer, sizeof(m_itemRenameBuffer), fn.c_str(), _TRUNCATE);
				m_startRenaming = true;
			}
			else {
				AX_CORE_LOG_ERROR("Failed to create folder: {}", ec.message());
			}
		}


		// ----- Draw search bar -----
		ImGui::SameLine();
		ImGui::SetNextItemWidth(200.0f);
		ImGui::InputTextWithHint("##Search", "Search...", m_searchBuffer, IM_ARRAYSIZE(m_searchBuffer));
		ImGui::SameLine();
		ImGui::BeginDisabled(m_searchBuffer[0] == '\0');
		if (ImGui::Button("X")) { m_searchBuffer[0] = '\0'; }
		ImGui::EndDisabled();

		ImGui::Separator();
	}

}
