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

	private:

		struct DirItem {
			std::filesystem::path path;
			bool isDir = false;
			std::string displayName;
		};

		// -- File system state --
		std::filesystem::path m_currentDirectory;
		std::filesystem::path m_rootDirectory;
		std::vector<DirItem> m_directoryEntries;

		// -- Search / filtering --
		char m_searchBuffer[128] = "";

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

		// -- Icons --
		Ref<Texture2D> m_folderIcon;
		Ref<Texture2D> m_fileIcon;
		Ref<Texture2D> m_backIcon;
		Ref<Texture2D> m_refreshIcon;
		Ref<Texture2D> m_addFolderIcon;

		// ----- Events -----
		bool onProjectChanged(ProjectChangedEvent& e);

		// -- Helper functions --
		void refreshDirectory();
		void resetRenaming();
		bool matchesSearch(const std::string& name);
		void deletePath(const std::filesystem::path& path);

		void drawToolbar();

	};

}
