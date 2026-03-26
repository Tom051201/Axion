#pragma once

namespace Axion {

	class Modal {
	public:

		Modal(const char* name, bool showTitle = false) : m_name(name), m_showTitle(showTitle) {}
		virtual ~Modal() = default;

		void onGuiRender();

		virtual void open();
		virtual void close();
		bool isOpen() const { return m_open; }

	protected:

		const char* m_name;
		bool m_showTitle;
		bool m_open = false;

		virtual void renderContent() = 0;
		virtual void resetInputs() {}

	};

}
