#pragma once

namespace Axion {

	class ContentBrowserPanel {
	public:

		ContentBrowserPanel();
		~ContentBrowserPanel();

		void setup();
		void shutdown();

		void onGuiRender();

	};

}
