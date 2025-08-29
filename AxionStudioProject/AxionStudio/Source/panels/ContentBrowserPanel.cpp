#include "ContentBrowserPanel.h"

#include "AxionEngine/Vendor/imgui/imgui.h"

#include "AxionEngine/Source/core/AssetManager.h"

namespace Axion {

	static const std::filesystem::path s_assetPath = "AxionStudio/Assets"; // TODO: make this part of a project config

	ContentBrowserPanel::ContentBrowserPanel() {

	}

	ContentBrowserPanel::~ContentBrowserPanel() {
		shutdown();
	}

	void ContentBrowserPanel::setup() {
		m_currentDirectory = s_assetPath;

		// TODO: create a editor own resource handler for such icons
		m_folderIcon = Texture2D::create("AxionStudio/Resources/contentbrowser/FolderIcon.png");
		m_fileIcon = Texture2D::create("AxionStudio/Resources/contentbrowser/FileIcon.png");
		m_backIcon = Texture2D::create("AxionStudio/Resources/contentbrowser/BackIcon.png");
		m_refreshIcon = Texture2D::create("AxionStudio/Resources/contentbrowser/RefreshIcon.png");
	}

	void ContentBrowserPanel::shutdown() {
		m_folderIcon->release();
		m_fileIcon->release();
		m_backIcon->release();
		m_refreshIcon->release();
	}

	void ContentBrowserPanel::onGuiRender() {
		ImGui::Begin("Content Browser");

		static float padding = 8.0f;
		static float thumbnailSize = 128.0f;

		// scrolling for zoom
		if (ImGui::IsWindowHovered(ImGuiHoveredFlags_AllowWhenBlockedByActiveItem)) {
			ImGuiIO& io = ImGui::GetIO();
			float scrollY = io.MouseWheel;
			if (io.KeyCtrl && scrollY != 0) {
				thumbnailSize += (io.KeyShift) ? scrollY : scrollY * 5.0f;
				thumbnailSize = std::clamp(thumbnailSize, 16.0f, 512.0f);
			}
		}

		// Tool bar
		ImGui::BeginDisabled(m_currentDirectory == std::filesystem::path(s_assetPath));
		if (ImGui::ImageButton("##Back_icon", reinterpret_cast<ImTextureID>(m_backIcon->getHandle()), { 30.0f, 30.0f })) {
			m_currentDirectory = m_currentDirectory.parent_path();
		}
		ImGui::EndDisabled();

		ImGui::SameLine();
		if (ImGui::ImageButton("##Refresh_icon", reinterpret_cast<ImTextureID>(m_refreshIcon->getHandle()), { 30.0f, 30.0f }, { 0, 1 }, { 1, 0 })) {
			// TODO: add refreshing, so that the files dont have to be gone thru every frame
		}

		ImGui::SameLine();
		ImGui::SetNextItemWidth(200.0f);
		ImGui::InputTextWithHint("##Search", "Search...", m_searchBuffer, IM_ARRAYSIZE(m_searchBuffer));

		// TODO: add sorting
		// TODO: show in explorer
		// TODO: add folder adding

		ImGui::Separator();

		float cellSize = thumbnailSize + padding;
			
		float panelWidth = ImGui::GetContentRegionAvail().x;
		int colCount = (int)(panelWidth / cellSize);
		if (colCount < 1) { colCount = 1; }
		ImGui::Columns(colCount, 0, false);

		for (auto& directoryEntry : std::filesystem::directory_iterator(m_currentDirectory)) {
			const auto& path = directoryEntry.path();
			auto relativePath = std::filesystem::relative(directoryEntry.path(), s_assetPath);
			std::string filenameString = relativePath.filename().string();

			// apply search filter
			if (m_searchBuffer[0] != '\0') {
				std::string searchString(m_searchBuffer);
				std::string lowerFileName = filenameString;
				std::string lowerSearch = searchString;

				std::transform(lowerFileName.begin(), lowerFileName.end(), lowerFileName.begin(), ::tolower);
				std::transform(lowerSearch.begin(), lowerSearch.end(), lowerSearch.begin(), ::tolower);

				if (lowerFileName.find(lowerSearch) == std::string::npos)
					continue;
			}

			Ref<Texture2D> icon = directoryEntry.is_directory() ? m_folderIcon : m_fileIcon;

			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
			std::string uniqueID = "##" + filenameString;
			if (ImGui::ImageButton((uniqueID + "_icon").c_str(), reinterpret_cast<ImTextureID>(icon->getHandle()), { thumbnailSize, thumbnailSize }, { 0, 1 }, { 1, 0 })) {
				if (directoryEntry.is_directory()) {
					// directory
					m_currentDirectory /= path.filename();
				}
				else {
					// file
				}
			}

			// drag and drop source
			if (ImGui::BeginDragDropSource()) {
				const std::string itemPath = relativePath.string();
				ImGui::SetDragDropPayload("CONTENT_BROWSER_ITEM", itemPath.c_str(), itemPath.size() + 1);
				ImGui::Image(reinterpret_cast<ImTextureID>(icon->getHandle()), { 30.0f, 30.0f }, { 0, 1 }, { 1, 0 });
				ImGui::EndDragDropSource();
			}

			ImGui::PopStyleColor();
			if (thumbnailSize >= 50) {
				ImGui::Text(filenameString.c_str());
			}

			ImGui::NextColumn();

		}
		
		
		ImGui::Columns(1);

		ImGui::End();
	}

}
