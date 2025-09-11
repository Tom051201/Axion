#pragma once
#include "axpch.h"

#include "AxionStudio/Source/core/Panel.h"

namespace Axion {

	class PanelManager {
	public:

		template<typename T, typename... Args>
		T* addPanel(Args&&... args) {
			auto panel = std::make_unique<T>(std::forward<Args>(args)...);
			T* raw = panel.get();
			m_panels.push_back(std::move(panel));
			return raw;
		}

		void setupAll();
		void shutdownAll();
		void renderAll();

		std::vector<Scope<Panel>>& getAllPanels() { return m_panels; }
		const std::vector<Scope<Panel>>& getAllPanels() const { return m_panels; }

		Panel* findPanelByName(const std::string& name);

	private:

		std::vector<Scope<Panel>> m_panels;

	};

}

