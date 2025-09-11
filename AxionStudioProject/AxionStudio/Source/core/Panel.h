#pragma once

#include <string>

namespace Axion {

	class Panel {
	public:

		Panel() = default;
		Panel(const std::string& name) : m_name(name) {}
		virtual ~Panel() = default;

		virtual void setup() = 0;
		virtual void shutdown() = 0;
		virtual void onGuiRender() = 0;

		virtual bool& isVisible() { return m_visible; }
		virtual const std::string& getName() const { return m_name; }

	protected:

		std::string m_name = "Unknown";
		bool m_visible = true;

	};

}
