#include "PanelManager.h"

namespace Axion {

	void PanelManager::setupAll() {
		for (auto& p : m_panels) {
			p->setup();
		}
	}

	void PanelManager::shutdownAll() {
		for (auto& p : m_panels) {
			p->shutdown();
		}
	}

	void PanelManager::renderAll() {
		for (auto& p : m_panels) {
			if (p->isVisible()) {
				p->onGuiRender();
			}
		}
	}

	void PanelManager::onEventAll(Event& e) {
		for (auto& p : m_panels) {
			p->onEvent(e);
		}
	}

	Panel* PanelManager::findPanelByName(const std::string& name) {
		auto it = std::find_if(m_panels.begin(), m_panels.end(),
			[&](auto& p) { return p->getName() == name; });
		return it != m_panels.end() ? it->get() : nullptr;
	}

}
