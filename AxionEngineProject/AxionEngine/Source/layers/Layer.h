#pragma once

#include "AxionEngine/Source/core/Core.h"
#include "AxionEngine/Source/core/Timestep.h"
#include "AxionEngine/Source/events/Event.h"

namespace Axion {

	class Layer {
	public:

		Layer(const std::string& name = "Layer") : m_debugname(name) {}
		virtual ~Layer() {}

		virtual void onAttach() {}
		virtual void onDetach() {}
		virtual void onUpdate(Timestep ts) {}
		virtual void onEvent(Event& e) {}
		virtual void onGuiRender() {}

		const std::string& getName() const { return m_debugname; }
		
		void setActive(bool active) { m_active = active; }
		bool isActive() { return m_active; }

	protected:

		std::string m_debugname;
		bool m_active = true;

		using EventCallbackFn = std::function<void(Event&)>;

	};

}
