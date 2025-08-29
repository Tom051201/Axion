#pragma once
#include "axpch.h"

#include "AxionEngine/Source/render/Texture.h"

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

		std::filesystem::path m_currentDirectory;

		char m_searchBuffer[128] = "";
		bool m_isDragging = false;

		Ref<Texture2D> m_folderIcon;
		Ref<Texture2D> m_fileIcon;
		Ref<Texture2D> m_backIcon;
		Ref<Texture2D> m_refreshIcon;

	};

}
