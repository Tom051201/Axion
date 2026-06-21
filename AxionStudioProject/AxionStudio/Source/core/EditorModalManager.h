#pragma once

#include "AxionStudio/Source/core/EditorActionQueue.h"

#include "AxionStudio/Vendor/Silica/include/SWidget.h"
#include "AxionStudio/Vendor/Silica/include/SBox.h"
#include "AxionStudio/Vendor/Silica/include/SOverlay.h"

namespace Axion {

	class EditorModalManager {
	public:

		static void initialize(std::shared_ptr<Silica::SBox> root, Silica::WidgetPtr mainLayout) {
			s_root = root;
			s_mainLayout = mainLayout;
		}

		static void shutdown() {
			s_root = nullptr;
			s_mainLayout = nullptr;
			s_currentModal = nullptr;
		}

		static void open(Silica::WidgetPtr modalWidget) {
			EditorActionQueue::push([modalWidget]() {
				s_currentModal = modalWidget;
				s_root->setChild(Silica::MakeWidget<Silica::SOverlay>({
					.children = { s_mainLayout, s_currentModal }
				}));
			});
		}

		static void close() {
			EditorActionQueue::push([]() {
				s_currentModal = nullptr;
				if (s_root && s_mainLayout) {
					s_root->setChild(s_mainLayout);
				}
			});
		}

	private:

		inline static std::shared_ptr<Silica::SBox> s_root = nullptr;
		inline static Silica::WidgetPtr s_mainLayout = nullptr;
		inline static Silica::WidgetPtr s_currentModal = nullptr;

	};

}
