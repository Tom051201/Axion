#pragma once
#include "axpch.h"

#include "AxionStudio/Source/core/Panel.h"

#include "AxionEngine/Source/render/Texture.h"
#include "AxionEngine/Source/project/Project.h"
#include "AxionEngine/Source/events/ApplicationEvent.h"

#include <filesystem>
#include <optional>

namespace Axion {

	class ContentBrowserPanel : public Panel {
	public:

		ContentBrowserPanel(const std::string& name);
		~ContentBrowserPanel() override;

		void setup() override;
		void shutdown() override;
		void onEvent(Event& e) override;
		void onGuiRender() override;

		void serialize(YAML::Emitter& out) const override;
		void deserialize(const YAML::Node& node) override;

		void refresh();

	private:

		// ----- Content browser item -----
		struct DirItem {
			std::filesystem::path path;
			bool isDir = false;
			std::string displayName;
		};

		// ----- Scenes overview item -----
		struct SceneNode {
			std::string name;
			std::filesystem::path path;
			bool isFolder = false;
			std::vector<SceneNode> children; // only valid if isFolder == true
		};

		// -- Content browser --
		std::filesystem::path m_currentDirectory;
		std::filesystem::path m_rootDirectory;
		std::vector<DirItem> m_directoryEntries;

		// -- Scenes overview --
		std::filesystem::path m_scenesDirectory;
		SceneNode m_scenesRootNode;

		// -- Search / filtering --
		char m_searchBuffer[128] = "";
		bool m_onlyEngineAssets = false; // TODO: add to state file

		// -- Renaming --
		std::filesystem::path m_itemBeingRenamed;
		char m_itemRenameBuffer[260] = "";
		bool m_startRenaming = false;
		std::optional<std::pair<std::filesystem::path, std::filesystem::path>> m_pendingRename;

		// -- Deleting --
		bool m_openDeletePopup = false;
		std::optional<std::filesystem::path> m_pendingDelete;

		// -- Navigating --
		std::optional<std::filesystem::path> m_pendingNavigate;

		// -- UI --
		bool m_showNames = true;
		float m_thumbnailSize = 128.0f;

		// -- Icons --
		Ref<Texture2D> m_folderIcon;
		Ref<Texture2D> m_fileIcon;
		Ref<Texture2D> m_backIcon;
		Ref<Texture2D> m_refreshIcon;
		Ref<Texture2D> m_addFolderIcon;

		// ----- Events -----
		bool onProjectChanged(ProjectChangedEvent& e);

		// -- Helper functions for content browser --
		void refreshDirectory();
		void resetRenaming();
		bool matchesSearch(const std::string& name);
		bool isEngineAssetExtension(const std::filesystem::path& path);
		void deletePath(const std::filesystem::path& path);

		// -- Helper functions for scenes overview --
		void refreshScenes();
		SceneNode scanSceneFolder(const std::filesystem::path& folderPath);
		void drawSceneNode(const SceneNode& node);

		void drawToolbar();

	};

}
