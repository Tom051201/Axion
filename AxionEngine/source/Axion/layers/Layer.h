#pragma once

#include "Axion/core/Core.h"
#include "Axion/core/Timestep.h"
#include "Axion/events/Event.h"

namespace Axion {

	class Layer {
	public:

		Layer(const std::string& name = "Layer");
		virtual ~Layer();

		virtual void onAttach() {}
		virtual void onDetach() {}
		virtual void onUpdate(Timestep ts) {}
		virtual void onEvent(Event& e) {}
		virtual void onGuiRender() {}

		inline const std::string& getName() const { return m_debugname; }

		bool m_active = true;

	protected:

		std::string m_debugname;

	};

}
