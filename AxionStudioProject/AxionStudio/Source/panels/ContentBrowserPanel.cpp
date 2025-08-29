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
		m_folderIcon = Texture2D::create("AxionStudio/Resources/folder.png");
		m_fileIcon = Texture2D::create("AxionStudio/Resources/file.png");
	}

	void ContentBrowserPanel::shutdown() {
		m_folderIcon->release();
		m_fileIcon->release();
	}

	void ContentBrowserPanel::onGuiRender() {
		ImGui::Begin("Content Browser");

		if (m_currentDirectory != std::filesystem::path(s_assetPath)) {
			if (ImGui::Button("<-")) {
				m_currentDirectory = m_currentDirectory.parent_path();
			}
		}

		static float padding = 8.0f;
		static float thumbnailSize = 128.0f;
		float cellSize = thumbnailSize + padding;
			
		float panelWidth = ImGui::GetContentRegionAvail().x;
		int colCount = (int)(panelWidth / cellSize);
		if (colCount < 1) { colCount = 1; }

		ImGui::Columns(colCount, 0, false);

		for (auto& directoryEntry : std::filesystem::directory_iterator(m_currentDirectory)) {
			const auto& path = directoryEntry.path();
			auto relativePath = std::filesystem::relative(directoryEntry.path(), s_assetPath);
			std::string filenameString = relativePath.filename().string();

			Ref<Texture2D> icon = directoryEntry.is_directory() ? m_folderIcon : m_fileIcon;

			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
			std::string uniqueID = "##" + filenameString;
			if (ImGui::ImageButton((uniqueID + "_icon").c_str(), reinterpret_cast<ImTextureID>(icon->getHandle()), { thumbnailSize, thumbnailSize }, { 0, 1 }, { 1, 0 })) {
				if (directoryEntry.is_directory()) {
					// directory
					m_currentDirectory /= path.filename();
				}
			}

			//if (ImGui::BeginDragDropSource()) {
			//	const std::wstring itemPath = relativePath.wstring();
			//	ImGui::SetDragDropPayload("CONTENT_BROWSER_ITEM", itemPath.c_str(), (itemPath.size() + 1) * sizeof(wchar_t));
			//	ImGui::Text("%s", filenameString.c_str()); // shows filename while dragging
			//	ImGui::EndDragDropSource();
			//}

			if (ImGui::BeginDragDropSource()) {
				const std::string itemPath = relativePath.string();
				ImGui::SetDragDropPayload("CONTENT_BROWSER_ITEM", itemPath.c_str(), itemPath.size() + 1);
				ImGui::Text("%s", filenameString.c_str()); // shows filename while dragging
				ImGui::EndDragDropSource();
			}

			ImGui::PopStyleColor();
			ImGui::Text(filenameString.c_str());

			ImGui::NextColumn();

		}
		
		
		ImGui::Columns(1);
		
		ImGui::SliderFloat("Icon Size", &thumbnailSize, 8.0f, 512.0f);
		ImGui::SliderFloat("Padding", &padding, 0.0f, 32.0f);

		ImGui::End();
	}

}
