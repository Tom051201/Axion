#pragma once

#include <unordered_set>
#include <functional>
#include <memory>

#include <Silica/include/SWidget.h>

#include "AxionEngine/Source/core/Core.h"
#include "AxionEngine/Source/scene/Scene.h"
#include "AxionEngine/Source/scene/Entity.h"
#include "AxionEngine/Source/events/ApplicationEvent.h"

#include "AxionStudio/Source/core/EditorEvents.h"

namespace Silica {
	class SBox;
	class SScrollBox;
	class SVerticalBox;
}

namespace Axion {

	class HierarchyPanel {
	public:

		HierarchyPanel() = default;
		~HierarchyPanel() = default;

		Silica::WidgetPtr getWidget();

		void onEvent(Event& ev);

		void rebuildUI();
		void setScene(Shared<Scene> scene);
		void refresh();

		void setEventCallback(std::function<void(Event&)> callback) { m_eventCallback = callback; }

	private:

		Silica::WidgetPtr buildEntityNode(Entity entity);

		Shared<Scene> m_scene;
		Entity m_selectedEntity = {};
		std::unordered_set<entt::entity> m_openNodes;

		std::function<void(Event&)> m_eventCallback;

		std::shared_ptr<Silica::SBox> m_uiRoot;
		std::shared_ptr<Silica::SVerticalBox> m_contentBox;

		EventReply onSceneChanged(SceneChangedEvent& ev);
		EventReply onEntitySelected(EntitySelectedEvent& ev);

	};

}
