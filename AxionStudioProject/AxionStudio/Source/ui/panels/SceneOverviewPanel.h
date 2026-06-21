#pragma once

#include "AxionEngine/Source/scene/Scene.h"
#include "AxionEngine/Source/events/ApplicationEvent.h"

#include "AxionStudio/Vendor/Silica/include/SWidget.h"
#include "AxionStudio/Vendor/Silica/include/SBox.h"
#include "AxionStudio/Vendor/Silica/include/FontAtlas.h"

#include <functional>

namespace Axion {

	class SceneOverviewPanel {
	public:

		SceneOverviewPanel() = default;
		~SceneOverviewPanel() = default;

		void onEvent(Event& e);

		void setScene(const Ref<Scene>& scene);

		Silica::WidgetPtr getWidget(Silica::FontAtlas* font);

	private:

		Ref<Scene> m_activeScene;

		std::shared_ptr<Silica::SBox> m_uiRoot;
		Silica::FontAtlas* m_font = nullptr;

		bool m_rebuildQueued = false;

		void rebuildUI();
		void rebuildUI_Internal();

		bool onSceneChanged(SceneChangedEvent& e);

	};

}
