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

		inline const std::string& getName() const { return m_debugname; }

	protected:

		std::string m_debugname;

		using EventCallbackFn = std::function<void(Event&)>;

	};

}
