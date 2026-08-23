#pragma once

#include <Silica/include/SWidget.h>

#include "AxionEngine/Source/events/Event.h"

#include "AxionStudio/Source/core/EditorEvents.h"

namespace Silica {
	class SVerticalBox;
}

namespace Axion {

	class HistoryPanel {
	public:

		~HistoryPanel() = default;

		Silica::WidgetPtr getWidget();
		void rebuildUI();

		void onEvent(Event& ev);

	private:

		Silica::WidgetPtr m_uiRoot;
		std::shared_ptr<Silica::SVerticalBox> m_contentBox;

		EventReply onEditorHistoryChanged(EditorHistoryChangedEvent& ev);

	};

}
