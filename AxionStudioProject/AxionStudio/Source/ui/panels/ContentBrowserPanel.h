#pragma once
#include "axpch.h"

#include "AxionEngine/Source/project/Project.h"
#include "AxionEngine/Source/events/ApplicationEvent.h"

#include "AxionEngine/Vendor/yaml-cpp/include/yaml-cpp/yaml.h"

#include "AxionStudio/Vendor/Silica/include/SWidget.h"
#include "AxionStudio/Vendor/Silica/include/SBox.h"
#include "AxionStudio/Vendor/Silica/include/SHorizontalSplitBox.h"
#include "AxionStudio/Vendor/Silica/include/SVerticalSplitBox.h"

#include "AxionStudio/Source/ui/modals/AudioImportModal.h"
#include "AxionStudio/Source/ui/modals/MaterialImportModal.h"
#include "AxionStudio/Source/ui/modals/MeshImportModal.h"
#include "AxionStudio/Source/ui/modals/PhysicsMaterialImportModal.h"
#include "AxionStudio/Source/ui/modals/PipelineImportModal.h"
#include "AxionStudio/Source/ui/modals/ShaderImportModal.h"
#include "AxionStudio/Source/ui/modals/SkyboxImportModal.h"
#include "AxionStudio/Source/ui/modals/Texture2DImportModal.h"
#include "AxionStudio/Source/ui/modals/TextureCubeImportModal.h"

#include "AxionStudio/Source/core/VirtualFileSystem.h"

namespace Axion {

	class ContentBrowser {
	public:

		ContentBrowser() = default;
		~ContentBrowser() = default;

		void setup();
		void onEvent(Event& e);

		void refresh();

		void loadSettings(const YAML::Node& editorConfig);
		void saveSettings(YAML::Emitter& out) const;

		void setOpenVisualScriptPanelCallback(const std::function<void(const std::filesystem::path& path)>& func) { m_openVisualScriptPanel = func; }
		void setAssetDeletedCallback(const std::function<void(const std::filesystem::path& path)>& func) { m_onAssetDeleted = func; }
		void setModalCallbacks(const std::function<void(Silica::WidgetPtr)>& open, const std::function<void()>& close) {
			m_openGlobalModal = open;
			m_closeGlobalModal = close;
		}
		void setAssetRenamedCallback(const std::function<void(const std::filesystem::path& oldPath, const std::filesystem::path& newPath)>& func) { m_onAssetRenamed = func; }
		void setOpenTextFileCallback(std::function<void(const std::filesystem::path&)> callback) { m_openTextFile = callback; }
		void setOpenMaterialPanelCallback(std::function<void(const std::filesystem::path&)> callback) { m_openMaterialPanel = callback; }

		Silica::WidgetPtr getWidget();

	private:

		struct DirItem {
			std::filesystem::path path;
			bool isDir = false;
			std::string displayName;
			std::shared_ptr<VFSNode> vfsNode = nullptr;
		};

		// -- Content browser --
		std::filesystem::path m_currentDirectory;
		std::filesystem::path m_rootDirectory;
		std::vector<DirItem> m_directoryEntries;
		bool m_showFileExtensions = true;
		std::unordered_set<std::string> m_expandedDirectories;
		Silica::WidgetPtr buildDirectoryTree(const std::filesystem::path& dirPath);
		float m_treeViewWidth = 220.0f;
		float m_treeViewTopHeight = 250.0f;
		std::shared_ptr<Silica::SHorizontalSplitBox> m_splitBox;
		std::shared_ptr<Silica::SVerticalSplitBox> m_vSplitBox;

		// -- VFS --
		VirtualFileSystem m_vfs;
		bool m_viewingCollection = false;
		std::shared_ptr<VFSNode> m_currentCollection;
		Silica::WidgetPtr buildCollectionTree(std::shared_ptr<VFSNode> node);

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
		float m_thumbnailSize = 100.0f;

		// -- Callbacks --
		std::function<void(const std::filesystem::path& path)> m_openVisualScriptPanel;
		std::function<void(const std::filesystem::path& path)> m_onAssetDeleted;
		std::function<void(Silica::WidgetPtr)> m_openGlobalModal;
		std::function<void()> m_closeGlobalModal;
		std::function<void(const std::filesystem::path& oldPath, const std::filesystem::path& newPath)> m_onAssetRenamed;
		std::function<void(const std::filesystem::path&)> m_openTextFile;
		std::function<void(const std::filesystem::path&)> m_openMaterialPanel;

		// -- Modals --
		std::shared_ptr<AudioImportModal> m_audioImportModal;
		std::shared_ptr<MaterialImportModal> m_materialImportModal;
		std::shared_ptr<MeshImportModal> m_meshImportModal;
		std::shared_ptr<PhysicsMaterialImportModal> m_physicsMaterialModal;
		std::shared_ptr<PipelineImportModal> m_pipelineImportModal;
		std::shared_ptr<ShaderImportModal> m_shaderImportModal;
		std::shared_ptr<SkyboxImportModal> m_skyboxImportModal;
		std::shared_ptr<Texture2DImportModal> m_texture2DImportModal;
		std::shared_ptr<TextureCubeImportModal> m_textureCubeImportModal;

		// -- Silica --
		bool m_rebuildQueued = false;
		std::shared_ptr<Silica::SBox> m_uiRoot;
		void rebuildUI();
		void rebuildUI_Internal();
		Silica::WidgetPtr buildToolbar();
		Silica::WidgetPtr buildContentArea();
		Silica::WidgetPtr buildDeleteModal();

		// -- Events --
		bool onProjectChanged(ProjectChangedEvent& e);

		// -- Helper functions --
		void refreshDirectory();
		void resetRenaming();
		bool matchesSearch(const std::string& name);
		void deletePath(const std::filesystem::path& path);
		std::vector<std::filesystem::path> findRelatedFiles(const std::filesystem::path& path);

	};

}
