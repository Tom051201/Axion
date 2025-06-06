#include "axpch.h"
#include "LayerStack.h"

namespace Axion {

	LayerStack::LayerStack() {
	}

	LayerStack::~LayerStack() {
		for (Layer* layer : m_layers) {
			layer->onDetach();
			delete layer;
		}
	}

	void LayerStack::pushLayer(Layer* layer) {
		m_layers.emplace(m_layers.begin() + m_layerInsertIndex, layer);
		m_layerInsertIndex++;
	}

	void LayerStack::pushOverlay(Layer* overlay) {
		m_layers.emplace_back(overlay);
	}

	void LayerStack::removeLayer(Layer* layer) {
		auto it = std::find(m_layers.begin(), m_layers.end(), layer);
		if (it != m_layers.end()) {
			m_layers.erase(it);
			m_layerInsertIndex--;
		}
	}

	void LayerStack::removeOverlay(Layer* overlay) {
		auto it = std::find(m_layers.begin(), m_layers.end(), overlay);
		if (it != m_layers.end()) {
			m_layers.erase(it);
		}
	}

}
