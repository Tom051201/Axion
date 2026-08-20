#pragma once

#include <memory>

#include <Silica/include/SWidget.h>

#include "AxionEngine/Source/events/ApplicationEvent.h"

namespace Silica {
	class SBox;
}

namespace Axion {

	class AssetManagerPanel {
	public:

		AssetManagerPanel() = default;
		~AssetManagerPanel() = default;

		Silica::WidgetPtr getWidget();

		void onEvent(Event& ev);

		void refresh();

	private:

		void rebuildUI_Internal();

		std::shared_ptr<Silica::SBox> m_uiRoot;
		bool m_rebuildQueued = false;

		EventReply onSceneChanged(SceneChangedEvent& ev);

	};

}
