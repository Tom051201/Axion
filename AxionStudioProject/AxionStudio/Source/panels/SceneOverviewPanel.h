#pragma once

#include "AxionEngine/Source/scene/Scene.h"
#include "AxionEngine/Source/events/ApplicationEvent.h"

#include "AxionStudio/Source/core/Panel.h"

namespace Axion {

	class SceneOverviewPanel : public Panel {
	public:

		SceneOverviewPanel(const std::string& name);
		~SceneOverviewPanel();

		void setup() override;
		void shutdown() override;
		void onEvent(Event& e) override;
		void onGuiRender() override;

		void setScene(const Ref<Scene>& scene) { m_activeScene = scene; }

	private:

		Ref<Scene> m_activeScene;

		// -- Title --
		char m_titleBuffer[128] = "";


		bool onSceneChanged(SceneChangedEvent& e);

	};

}
