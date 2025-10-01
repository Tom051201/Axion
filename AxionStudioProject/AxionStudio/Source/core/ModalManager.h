#pragma once
#include "axpch.h"

#include "AxionStudio/Source/core/Modal.h"

namespace Axion {

	class ModalManager {
	public:

		template<typename T, typename... Args>
		T* addModal(Args&&... args) {
			auto modal = std::make_unique<T>(std::forward<Args>(args)...);
			T* raw = modal.get();
			m_modals.push_back(std::move(modal));
			return raw;
		}

		void renderAll();

		std::vector<Scope<Modal>>& getAllPanels() { return m_modals; }
		const std::vector<Scope<Modal>>& getAllPanels() const { return m_modals; }

	private:

		std::vector<Scope<Modal>> m_modals;

	};

}
