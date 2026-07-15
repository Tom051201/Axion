#pragma once

#include "AxionEngine/Source/scene/Scene.h"
#include "AxionEngine/Source/events/ApplicationEvent.h"

#include "AxionStudio/Vendor/Silica/include/SBox.h"

namespace Axion {

	class SceneOverviewPanel {
	public:

		SceneOverviewPanel() = default;
		~SceneOverviewPanel() = default;

		void onEvent(Event& e);

		Silica::WidgetPtr getWidget();

		void rebuildUI();
		void setScene(const Ref<Scene>& scene);

	private:

		Ref<Scene> m_activeScene;
		std::shared_ptr<Silica::SBox> m_uiRoot;
		bool m_rebuildQueued = false;

		void rebuildUI_Internal();
		bool onSceneChanged(SceneChangedEvent& e);

	};

}
