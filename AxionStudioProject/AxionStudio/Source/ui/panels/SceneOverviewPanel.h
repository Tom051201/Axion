#pragma once

#include <memory>

#include <Silica/include/SWidget.h>

#include "AxionEngine/Source/core/Core.h"
#include "AxionEngine/Source/scene/Scene.h"
#include "AxionEngine/Source/events/ApplicationEvent.h"

#include "AxionStudio/Source/core/EditorEvents.h"

namespace Silica {
	class SBox;
}

namespace Axion {

	class SceneOverviewPanel {
	public:

		SceneOverviewPanel() = default;
		~SceneOverviewPanel() = default;

		Silica::WidgetPtr getWidget();

		void onEvent(Event& e);
		void setEventCallback(std::function<void(Event&)> callback) { m_eventCallback = callback; }

		void rebuildUI();
		void setScene(const Shared<Scene>& scene);

	private:

		Shared<Scene> m_activeScene;
		std::shared_ptr<Silica::SBox> m_uiRoot;
		bool m_rebuildQueued = false;
		std::function<void(Event&)> m_eventCallback;

		void rebuildUI_Internal();

		EventReply onSceneChanged(SceneChangedEvent& ev);
		EventReply onSceneModified(SceneModifiedEvent& ev);

	};

}
