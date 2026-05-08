#pragma once
#include "axpch.h"

#include "AxionStudio/Source/core/Panel.h"

#include "AxionEngine/Source/project/Project.h"
#include "AxionEngine/Source/events/ApplicationEvent.h"

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

		void setOpenVisualScriptPanelCallback(const std::function<void(const std::filesystem::path& path)>& func) { m_openVisualScriptPanel = func; }
		void setAssetDeletedCallback(const std::function<void(const std::filesystem::path& path)>& func) { m_onAssetDeleted = func; }

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
		bool m_showFileExtensions = true;

		// -- Search / filtering --
		std::string m_searchString;
		bool m_onlyEngineAssets = false;

		// -- Renaming --
		std::filesystem::path m_itemBeingRenamed;
		std::string m_itemRenameString;
		bool m_startRenaming = false;
		std::optional<std::pair<std::filesystem::path, std::filesystem::path>> m_pendingRename;

		// -- Deleting --
		bool m_openDeletePopup = false;
		std::optional<std::filesystem::path> m_pendingDelete;
		std::vector<std::filesystem::path> m_relatedFilesToDelete;
		bool m_deleteRelatedFiles = true;

		// -- Navigating --
		std::optional<std::filesystem::path> m_pendingNavigate;
		std::vector<std::filesystem::path> m_backHistory;
		std::vector<std::filesystem::path> m_forwardHistory;

		// -- UI --
		bool m_showNames = true;
		float m_thumbnailSize = 128.0f;

		// -- Callbacks --
		std::function<void(const std::filesystem::path& path)> m_openVisualScriptPanel;
		std::function<void(const std::filesystem::path& path)> m_onAssetDeleted;

		// ----- Events -----
		bool onProjectChanged(ProjectChangedEvent& e);

		// -- Helper functions for content browser --
		void refreshDirectory();
		void resetRenaming();
		bool matchesSearch(const std::string& name);
		bool isEngineAssetExtension(const std::filesystem::path& path);
		void deletePath(const std::filesystem::path& path);
		std::vector<std::filesystem::path> findRelatedFiles(const std::filesystem::path& path);

		void drawToolbar();

	};

}
