#pragma once

#include <memory>
#include <filesystem>
#include <functional>
#include <string>
#include <unordered_set>
#include <vector>
#include <optional>
#include <utility>

#include <Silica/include/SWidget.h>

#include "AxionEngine/Source/project/Project.h"
#include "AxionEngine/Source/events/ApplicationEvent.h"

#include "AxionStudio/Source/core/VirtualFileSystem.h"
#include "AxionStudio/Source/core/EditorActionQueue.h"
#include "AxionStudio/Source/core/EditorModalManager.h"
#include "AxionStudio/Source/core/EditorEvents.h"

namespace YAML {
	class Node;
	class Emitter;
}

namespace Silica {
	class SBox;
	class SHorizontalSplitBox;
	class SVerticalSplitBox;
	class SEditableText;
	class SMenuAnchor;
}

namespace Axion {
	class AudioImportModal;
	class MaterialImportModal;
	class MeshImportModal;
	class PhysicsMaterialImportModal;
	class PipelineImportModal;
	class ShaderImportModal;
	class SkyboxImportModal;
	class Texture2DImportModal;
	class TextureCubeImportModal;
}

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

		void setEventCallback(std::function<void(Event&)> callback) { m_eventCallback = callback; }
		void setOpenVisualScriptPanelCallback(const std::function<void(const std::filesystem::path& path)>& callback) { m_openVisualScriptPanel = callback; }
		void setOpenTextEditorPanelCallback(std::function<void(const std::filesystem::path&)> callback) { m_openTextEditorPanel = callback; }
		void setOpenMaterialEditorPanelCallback(std::function<void(const std::filesystem::path&)> callback) { m_openMaterialEditorPanel = callback; }
		void setOpenSceneInViewportCallback(std::function<void(const std::filesystem::path&)> callback) { m_openSceneInViewportCallback = callback; }

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
		float m_treeViewWidth = 220.0f;
		float m_treeViewTopHeight = 250.0f;
		std::shared_ptr<Silica::SHorizontalSplitBox> m_splitBox;
		std::shared_ptr<Silica::SVerticalSplitBox> m_vSplitBox;

		// -- VFS --
		VirtualFileSystem m_vfs;
		bool m_viewingCollection = false;
		std::shared_ptr<VFSNode> m_currentCollection;

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
		bool m_showContentArea = true;
		bool m_showVFSTree = true;
		bool m_showPhysicalTree = true;
		float m_thumbnailSize = 100.0f;
		std::shared_ptr<Silica::SEditableText> m_searchBoxWidget;

		// -- Callbacks --
		std::function<void(Event&)> m_eventCallback;
		std::function<void(const std::filesystem::path& path)> m_openVisualScriptPanel;
		std::function<void(const std::filesystem::path&)> m_openTextEditorPanel;
		std::function<void(const std::filesystem::path&)> m_openMaterialEditorPanel;
		std::function<void(const std::filesystem::path&)> m_openSceneInViewportCallback;

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

		// -- UI Builders --
		Silica::WidgetPtr buildToolbar();
		Silica::WidgetPtr buildContentArea();
		Silica::WidgetPtr buildDeleteModal();
		Silica::WidgetPtr buildDirectoryTree(const std::filesystem::path& dirPath);
		Silica::WidgetPtr buildCollectionTree(std::shared_ptr<VFSNode> node);
		Silica::WidgetPtr createAssetItemWidget(const DirItem& item);

		std::shared_ptr<Silica::SMenuAnchor> buildCreateAssetSubMenu();
		Silica::WidgetPtr buildBackgroundContextMenu(Silica::WidgetPtr createAssetSubMenuWidget);

		// -- Commands --
		void cmdCreateCollection(std::shared_ptr<VFSNode> parentNode);
		void cmdDeleteCollection(std::shared_ptr<VFSNode> node);
		void cmdCreateFolder();
		void cmdCreateVisualScript();
		void cmdRenameItem(const std::filesystem::path& path);
		void cmdDeleteItem(const std::filesystem::path& path, std::shared_ptr<VFSNode> vfsNode);
		void cmdOpenItem(const std::filesystem::path& path, bool isDir, std::shared_ptr<VFSNode> vfsNode);

		template<typename TModal, typename SetupFunc = std::nullptr_t>
		void openAssetModal(std::shared_ptr<TModal> ContentBrowser::* modalMember, SetupFunc preSetup = nullptr) {
			EditorActionQueue::push([this, modalMember, preSetup]() {
				auto& modalPtr = this->*modalMember;
				modalPtr = std::make_shared<TModal>();

				if constexpr (!std::is_same_v<SetupFunc, std::nullptr_t>) {
					preSetup(modalPtr);
				}

				auto modalWidget = modalPtr->getWidget([this, modalMember]() {
					EditorActionQueue::push([this, modalMember]() {
						EditorModalManager::close();
						(this->*modalMember) = nullptr;
						refresh();
					});
				});

				EditorModalManager::open(modalWidget);
			});
		}

		// -- Events & Helpers --
		EventReply onProjectChanged(ProjectChangedEvent& e);
		EventReply onEditorSettingsChanged(EditorSettingsChangedEvent& ev);
		void refreshDirectory();
		void resetRenaming();
		bool matchesSearch(const std::string& name);
		void deletePath(const std::filesystem::path& path);
		std::vector<std::filesystem::path> findRelatedFiles(const std::filesystem::path& path);

	};

}
