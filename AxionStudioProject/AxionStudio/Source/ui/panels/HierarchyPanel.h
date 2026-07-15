#pragma once

#include <unordered_set>

#include "AxionStudio/Vendor/Silica/include/SBox.h"
#include "AxionStudio/Vendor/Silica/include/SScrollBox.h"
#include "AxionStudio/Vendor/Silica/include/SVerticalBox.h"

#include "AxionEngine/Source/scene/Scene.h"
#include "AxionEngine/Source/scene/Entity.h"

namespace Axion {

	class HierarchyPanel {
	public:

		HierarchyPanel() = default;
		~HierarchyPanel() = default;

		Silica::WidgetPtr getWidget();

		void rebuildUI();
		void setScene(Ref<Scene> scene);
		void refresh();

		void setSelectionCallback(std::function<void(Entity)> callback);

	private:

		Silica::WidgetPtr buildEntityNode(Entity entity);

		Ref<Scene> m_scene;
		std::function<void(Entity)> m_onEntitySelected;
		Entity m_selectedEntity = {};
		std::unordered_set<entt::entity> m_openNodes;

		std::shared_ptr<Silica::SBox> m_uiRoot;
		std::shared_ptr<Silica::SVerticalBox> m_contentBox;

	};

}
