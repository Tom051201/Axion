#include "studiopch.h"
#include "EditorModalManager.h"

#include <Silica/include/SBox.h>
#include <Silica/include/SOverlay.h>

#include "AxionStudio/Source/core/EditorActionQueue.h"

namespace Axion {

	void EditorModalManager::initialize(std::shared_ptr<Silica::SBox> root, Silica::WidgetPtr mainLayout) {
		s_root = root;
		s_mainLayout = mainLayout;
	}

	void EditorModalManager::shutdown() {
		s_root = nullptr;
		s_mainLayout = nullptr;
		s_currentModal = nullptr;
	}

	void EditorModalManager::open(Silica::WidgetPtr modalWidget) {
		EditorActionQueue::push([modalWidget]() {
			s_currentModal = modalWidget;
			s_root->setChild(Silica::MakeWidget<Silica::SOverlay>({
				.children = { s_mainLayout, s_currentModal }
			}));
		});
	}

	void EditorModalManager::close() {
		EditorActionQueue::push([]() {
			s_currentModal = nullptr;
			if (s_root && s_mainLayout) {
				s_root->setChild(s_mainLayout);
			}
		});
	}

}
