#pragma once

#include <memory>

#include <Silica/include/SWidget.h> 

namespace Silica {
	class SBox;
}

namespace Axion {

	class EditorModalManager {
	public:

		static void initialize(std::shared_ptr<Silica::SBox> root, Silica::WidgetPtr mainLayout);
		static void shutdown();
		static void open(Silica::WidgetPtr modalWidget);
		static void close();

	private:

		inline static std::shared_ptr<Silica::SBox> s_root = nullptr;
		inline static Silica::WidgetPtr s_mainLayout = nullptr;
		inline static Silica::WidgetPtr s_currentModal = nullptr;

	};

}
