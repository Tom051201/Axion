#pragma once
#include "Layer.h"

#include <vector>

namespace Axion {

	class LayerStack {
	public:

		using LayerList = std::vector<Layer*>;

		LayerStack();
		~LayerStack();

		void clear();

		void pushLayer(Layer* layer);
		void pushOverlay(Layer* overlay);
		void removeLayer(Layer* layer);
		void removeOverlay(Layer* overlay);

		LayerList::iterator begin() { return m_layers.begin(); }
		LayerList::iterator end() { return m_layers.end(); }

	private:

		LayerList m_layers;
		uint32_t m_layerInsertIndex = 0;

	};

}
