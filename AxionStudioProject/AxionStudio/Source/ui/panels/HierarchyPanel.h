#pragma once

#include <unordered_set>
#include <functional>
#include <memory>

#include <Silica/include/SWidget.h>

#include "AxionEngine/Source/core/Core.h"
#include "AxionEngine/Source/scene/Scene.h"
#include "AxionEngine/Source/scene/Entity.h"

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

		void rebuildUI();
		void setScene(Shared<Scene> scene);
		void refresh();

		void setSelectionCallback(std::function<void(Entity)> callback);

	private:

		Silica::WidgetPtr buildEntityNode(Entity entity);

		Shared<Scene> m_scene;
		std::function<void(Entity)> m_onEntitySelected;
		Entity m_selectedEntity = {};
		std::unordered_set<entt::entity> m_openNodes;

		std::shared_ptr<Silica::SBox> m_uiRoot;
		std::shared_ptr<Silica::SVerticalBox> m_contentBox;

	};

}
