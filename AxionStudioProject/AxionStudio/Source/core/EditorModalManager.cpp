#include "studiopch.h"
#include "EditorModalManager.h"

#include <Silica/include/SBox.h>
#include <Silica/include/SOverlay.h>

#include "AxionStudio/Source/core/EditorActionQueue.h"

namespace Axion {

	void EditorModalManager::shutdown() {
		close();
	}

	void EditorModalManager::open(Silica::WidgetPtr modalWidget) {
		EditorActionQueue::push([modalWidget]() {
			if (s_activeModalID != 0) {
				Silica::Renderer::closeOverlay(s_activeModalID);
			}

			Silica::Geometry emptyGeo = { {0.0f, 0.0f}, {0.0f, 0.0f} };
			s_activeModalID = Silica::Renderer::openModal(modalWidget, emptyGeo, []() {
				s_activeModalID = 0;
			});

		});
	}

	void EditorModalManager::close() {
		EditorActionQueue::push([]() {
			if (s_activeModalID != 0) {
				Silica::Renderer::closeOverlay(s_activeModalID);
				s_activeModalID = 0;
			}
		});
	}

}
