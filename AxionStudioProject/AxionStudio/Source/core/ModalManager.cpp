#include "ModalManager.h"

namespace Axion {

	void ModalManager::renderAll() {
		for (auto& m : m_modals) {
			if (m->isOpen()) {
				m->onGuiRender();
			}
		}
	}

}
