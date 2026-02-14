#include "ContentBrowserPanel.h"

#include "AxionEngine/Vendor/imgui/imgui.h"

#include "AxionEngine/Source/core/AssetManager.h"
#include "AxionEngine/Source/core/PlatformUtils.h"
#include "AxionEngine/Source/project/ProjectManager.h"
#include "AxionEngine/Source/scene/SceneManager.h"

#include "AxionStudio/Source/core/EditorResourceManager.h"

namespace Axion {

	constexpr float iconSize = 30.0f;

	ContentBrowserPanel::ContentBrowserPanel(const std::string& name) : Panel(name) {}

	ContentBrowserPanel::~ContentBrowserPanel() {
		shutdown();
	}

	void ContentBrowserPanel::setup() {

		if (ProjectManager::hasProject()) {
			m_rootDirectory = ProjectManager::getProject()->getAssetsPath();
			m_currentDirectory = m_rootDirectory;
			refreshDirectory();
		}

		EditorResourceManager::loadIcon("FolderIcon", "AxionStudio/Resources/contentbrowser/FolderIcon.png");
		EditorResourceManager::loadIcon("FileIcon", "AxionStudio/Resources/contentbrowser/FileIcon.png");
		EditorResourceManager::loadIcon("BackIcon", "AxionStudio/Resources/contentbrowser/BackIcon.png");
		EditorResourceManager::loadIcon("RefreshIcon", "AxionStudio/Resources/contentbrowser/RefreshIcon.png");
		EditorResourceManager::loadIcon("AddFolderIcon", "AxionStudio/Resources/contentbrowser/AddFolderIcon.png");

		refreshDirectory();
	}

	void ContentBrowserPanel::shutdown() {}

	void ContentBrowserPanel::onGuiRender() {
		ImGui::Begin("Content Browser");

		static float padding = 8.0f;

		// ----- Draw info when no project is selected -----
		if (!ProjectManager::hasProject()) {
			ImGui::TextWrapped("No Project Loaded. \nPlease load or create a project first.");
			ImGui::End();
			return;
		}


		// ----- Draw toolbar -----
		drawToolbar();


		// ----- Draw ContentBrowser -----
		float cellSize = m_thumbnailSize + padding;
		float panelWidth = ImGui::GetContentRegionAvail().x - 200.0f;
		panelWidth = std::max(0.0f, panelWidth);
		int colCount = (int)(panelWidth / cellSize);
		if (colCount < 1) { colCount = 1; }
		ImGui::BeginChild("ContentBrowserChild", ImVec2(panelWidth, ImGui::GetContentRegionAvail().y));	// ContentBrowserChild begin


		// ----- Scrolling for zoom (only inside the content area) -----
		if (ImGui::IsWindowHovered(ImGuiHoveredFlags_AllowWhenBlockedByActiveItem)) {
			ImGuiIO& io = ImGui::GetIO();
			float scrollY = io.MouseWheel;
			if (io.KeyCtrl && scrollY != 0) {
				m_thumbnailSize += (io.KeyShift) ? scrollY : scrollY * 5.0f;
				m_thumbnailSize = std::clamp(m_thumbnailSize, 16.0f, 512.0f);
				m_showNames = m_thumbnailSize >= 50;
			}
		}


		// ----- Draw content -----
		ImGui::Columns(colCount, 0, false);
		for (const auto& item : m_directoryEntries) {
			const auto& path = item.path;
			const std::string& filenameString = item.displayName;
			// -- Setup icon --
			Ref<Texture2D> icon = item.isDir ? EditorResourceManager::getIcon("FolderIcon") : EditorResourceManager::getIcon("FileIcon");

			// -- Apply search filter --
			if (!matchesSearch(filenameString)) continue;

			// -- Apply "Only Engine Assets" filter --
			if (m_onlyEngineAssets && !item.isDir && !isEngineAssetExtension(path)) continue;

			// -- Draw file / folder icon --
			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
			std::string uniqueID = "##" + path.string();
			if (ImGui::ImageButton((uniqueID + "_icon").c_str(), reinterpret_cast<ImTextureID>(icon->getHandle()), { m_thumbnailSize, m_thumbnailSize }, { 0, 1 }, { 1, 0 })) {
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
				if (item.path.string().find(".axshader") != std::string::npos) {
					if (ImGui::MenuItem("Recompile")) {
						AssetHandle<Shader> handle = AssetManager::load<Shader>(item.path.string());
						AssetManager::get<Shader>(handle)->recompile();
					}
				}

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
					ImGui::SetNextItemWidth(m_thumbnailSize);
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
					std::filesystem::path p(filenameString);
					std::string displayName = m_showFileExtensions ? p.filename().string() : p.stem().string();

					ImVec2 textSize = ImGui::CalcTextSize(displayName.c_str());
					float textX = std::max(ImGui::GetCursorPosX(), ImGui::GetCursorPosX() + (m_thumbnailSize - textSize.x) * 0.5f);
					ImGui::SetCursorPosX(textX);
					ImGui::TextUnformatted(displayName.c_str());
				}
			}

			ImGui::NextColumn();

		}
		ImGui::Columns(1);
		ImGui::EndChild();	// ContentBrowserChild end


		// ---- Vertical separator -----
		ImGui::SameLine();
		ImGui::InvisibleButton("vspe", ImVec2(1, ImGui::GetContentRegionAvail().y));
		if (ImGui::IsItemVisible()) {
			ImDrawList* drawList = ImGui::GetWindowDrawList();
			ImVec2 p0 = ImGui::GetItemRectMin();
			ImVec2 p1 = ImGui::GetItemRectMax();
			drawList->AddLine(p0, p1, ImGui::GetColorU32(ImGuiCol_Separator));
		}
		ImGui::SameLine();


		// ----- Scenes overview -----
		ImGui::SameLine();
		ImGui::BeginChild("ScenesChild", ImVec2(200.0f, ImGui::GetContentRegionAvail().y));	// ScenesChild begin
		ImGui::Text("Scenes");
		ImGui::Separator();

		for (const auto& child : m_scenesRootNode.children) {
			drawSceneNode(child);
		}

		ImGui::EndChild();	// ScenesChild end


		// ----- Execute renaming after the loop -----
		if (m_pendingRename.has_value()) {
			const auto& [oldPath, newPath] = *m_pendingRename;
			std::error_code ec;
			// -- Final safety checks --
			if (!std::filesystem::exists(newPath, ec)) {
				std::filesystem::rename(oldPath, newPath, ec);
				if (ec) { AX_CORE_LOG_ERROR("Rename failed: {}", ec.message()); }
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
				refresh();
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

	void ContentBrowserPanel::onEvent(Event& e) {
		EventDispatcher dispatcher(e);
		dispatcher.dispatch<ProjectChangedEvent>(AX_BIND_EVENT_FN(ContentBrowserPanel::onProjectChanged));
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

	bool ContentBrowserPanel::isEngineAssetExtension(const std::filesystem::path& path) {
		auto ext = path.extension().string();
		std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
		return ext == ".axmesh" || ext == ".axsky" || ext == ".axshader" || ext == ".axmat" || ext == ".axaudio" || ext == ".axtex";
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
		if (ImGui::ImageButton("##Back_icon", reinterpret_cast<ImTextureID>(EditorResourceManager::getIcon("BackIcon")->getHandle()), {iconSize, iconSize})) {
			m_currentDirectory = m_currentDirectory.parent_path();
			refreshDirectory();
		}
		ImGui::EndDisabled();


		// ----- Refresh button -----
		ImGui::SameLine();
		if (ImGui::ImageButton("##Refresh_icon", reinterpret_cast<ImTextureID>(EditorResourceManager::getIcon("RefreshIcon")->getHandle()), { iconSize, iconSize }, { 0, 1 }, { 1, 0 })) {
			refresh();
		}


		// ----- Add folder button -----
		ImGui::SameLine();
		if (ImGui::ImageButton("##AddFolder_icon", reinterpret_cast<ImTextureID>(EditorResourceManager::getIcon("AddFolderIcon")->getHandle()), { iconSize, iconSize }, { 0, 1 }, { 1, 0 })) {
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


		// ----- Only engine assets toggle -----
		ImGui::SameLine();
		ImGui::Checkbox("##OnlyEngineAssets_checkbox", &m_onlyEngineAssets);
		if (ImGui::IsItemHovered()) {
			ImGui::SetTooltip("Show only Asset files");
		}


		// ----- Show file extensions toggle -----
		ImGui::SameLine();
		ImGui::Checkbox("##ShowFileExtensions_checkbox", &m_showFileExtensions);
		if (ImGui::IsItemHovered()) {
			ImGui::SetTooltip("Show file extensions");
		}

		ImGui::Separator();
	}

	bool ContentBrowserPanel::onProjectChanged(ProjectChangedEvent& e) {
		if (ProjectManager::hasProject()) {
			std::error_code ec;
			// -- Content browser --
			m_rootDirectory = std::filesystem::weakly_canonical(ProjectManager::getProject()->getAssetsPath(), ec);
			if (ec) {
				m_rootDirectory = std::filesystem::absolute(ProjectManager::getProject()->getAssetsPath(), ec);
			}
			m_currentDirectory = m_rootDirectory;

			// -- Scenes overview --
			m_scenesDirectory = ProjectManager::getProject()->getScenesPath();

			refresh();
		}
		else {
			m_directoryEntries.clear();
			m_rootDirectory.clear();
			m_currentDirectory.clear();
		}

		return false;
	}

	void ContentBrowserPanel::serialize(YAML::Emitter& out) const {
		Panel::serialize(out);
		out << YAML::Key << "ThumbnailSize" << YAML::Value << m_thumbnailSize;
		out << YAML::Key << "OnlyAssetFiles" << YAML::Value << m_onlyEngineAssets;
		out << YAML::Key << "ShowExtensions" << YAML::Value << m_showFileExtensions;
	}

	void ContentBrowserPanel::deserialize(const YAML::Node& node) {
		Panel::deserialize(node);
		if (node["ThumbnailSize"]) {
			m_thumbnailSize = node["ThumbnailSize"].as<float>();
			m_showNames = m_thumbnailSize >= 50;
		}
		if (node["OnlyAssetFiles"]) {
			m_onlyEngineAssets = node["OnlyAssetFiles"].as<bool>();
		}
		if (node["ShowExtensions"]) {
			m_showFileExtensions = node["ShowExtensions"].as<bool>();
		}
	}

	void ContentBrowserPanel::refreshScenes() {
		if (std::filesystem::exists(m_scenesDirectory)) {
			m_scenesRootNode = scanSceneFolder(m_scenesDirectory);
		}
		else {
			m_scenesRootNode = {};
		}
	}

	ContentBrowserPanel::SceneNode ContentBrowserPanel::scanSceneFolder(const std::filesystem::path& folderPath) {
		SceneNode node;
		node.name = folderPath.filename().string();
		node.path = folderPath;
		node.isFolder = true;

		std::error_code ec;
		for (auto& entry : std::filesystem::directory_iterator(folderPath, std::filesystem::directory_options::skip_permission_denied, ec)) {
			if (ec) { ec.clear(); continue; }

			SceneNode child;
			if (entry.is_directory(ec)) {
				child = scanSceneFolder(entry.path());
			}
			else if (entry.is_regular_file(ec) && entry.path().extension() == ".axscene") {
				child.name = entry.path().filename().stem().string();
				child.path = entry.path();
				child.isFolder = false;
			}
			else continue;

			node.children.push_back(std::move(child));
		}

		return node;
	}

	void ContentBrowserPanel::drawSceneNode(const SceneNode& node) {
		ImVec2 iconSize{ 16, 16 };
		float verticalSpacing = 2.0f;

		if (node.isFolder) {
			ImGui::PushID(node.path.string().c_str());

			// -- Icon --
			ImGui::Image(reinterpret_cast<ImTextureID>(EditorResourceManager::getIcon("FolderIcon")->getHandle()), iconSize, { 0, 1 }, { 1, 0 });
			ImGui::SameLine();

			if (ImGui::TreeNodeEx(node.name.c_str(), ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth)) {
				// -- Spacing --
				ImGui::Dummy(ImVec2(0.0f, verticalSpacing));

				// -- Draw children --
				for (const auto& child : node.children) {
					drawSceneNode(child);
				}
				ImGui::TreePop();
			}

			ImGui::PopID();

			// -- Spacing --
			ImGui::Dummy(ImVec2(0.0f, verticalSpacing));
		}
		else {
			ImGui::Image(reinterpret_cast<ImTextureID>(EditorResourceManager::getIcon("FileIcon")->getHandle()), iconSize, { 0, 1 }, { 1, 0 });
			ImGui::SameLine();
			bool isTheSame = node.path.string() == SceneManager::getScenePath();
			if (ImGui::Selectable(node.name.c_str(), isTheSame)) {
				if (!isTheSame) SceneManager::loadScene(node.path.string());
			}


			// -- Draw options on right click --
			if (ImGui::BeginPopupContextItem(node.name.c_str())) {
				// -- Show in explorer button --
				if (ImGui::MenuItem("Show in Explorer")) {
					PlatformUtils::showInFileExplorer(node.path.string());
				}

				if (ImGui::MenuItem("Set as default Scene")) {
					ProjectManager::getProject()->setDefaultScene(node.path.string());
				}

				// -- Deleting --
				ImGui::Separator();
				if (ImGui::MenuItem("Delete")) {
					m_pendingDelete = node.path;
					m_openDeletePopup = true;
				}

				ImGui::EndPopup();
			}


			// -- Spacing --
			ImGui::Dummy(ImVec2(0.0f, verticalSpacing));
		}
	}

	void ContentBrowserPanel::refresh() {
		refreshDirectory();
		refreshScenes();
	}

}
