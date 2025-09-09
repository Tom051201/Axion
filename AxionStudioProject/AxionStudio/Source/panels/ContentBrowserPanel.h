#pragma once
#include "axpch.h"

#include "AxionEngine/Source/render/Texture.h"
#include "AxionEngine/Source/project/Project.h"

#include <filesystem>

namespace Axion {

	class ContentBrowserPanel {
	public:

		ContentBrowserPanel();
		~ContentBrowserPanel();

		void setup();
		void shutdown();

		void onGuiRender();

	private:

		struct DirItem {
			std::filesystem::path path;
			bool isDir = false;
			std::string displayName;
		};

		std::filesystem::path m_currentDirectory;

		char m_searchBuffer[128] = "";
		bool m_isDragging = false;

		std::vector<DirItem> m_directoryEntries;
		std::filesystem::path m_rootDirectory;

		Ref<Texture2D> m_folderIcon;
		Ref<Texture2D> m_fileIcon;
		Ref<Texture2D> m_backIcon;
		Ref<Texture2D> m_refreshIcon;

		Ref<Project> m_activeProject;

		void refreshDirectory();

	};

}
