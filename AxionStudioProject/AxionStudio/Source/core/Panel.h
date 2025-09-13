#pragma once

#include "AxionEngine/Vendor/yaml-cpp/include/yaml-cpp/yaml.h"

#include "AxionEngine/Source/events/Event.h"

#include <string>

namespace Axion {

	class Panel {
	public:

		Panel() = default;
		Panel(const std::string& name) : m_name(name) {}
		virtual ~Panel() = default;

		virtual void setup() = 0;
		virtual void shutdown() = 0;
		virtual void onEvent(Event& e) {}
		virtual void onGuiRender() = 0;

		// -- De-/Serialize common values --
		virtual void serialize(YAML::Emitter& out) const {
			out << YAML::Key << "Name" << YAML::Value << m_name;
			out << YAML::Key << "Visible" << YAML::Value << m_visible;
		}
		virtual void deserialize(const YAML::Node& node) {
			if (node["Visible"]) m_visible = node["Visible"].as<bool>();
		}

		virtual bool& isVisible() { return m_visible; }
		virtual const std::string& getName() const { return m_name; }

	protected:

		std::string m_name = "Unknown";
		bool m_visible = true;

	};

}
