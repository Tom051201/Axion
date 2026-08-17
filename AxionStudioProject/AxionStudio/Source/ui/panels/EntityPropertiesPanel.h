#pragma once

#include <functional>
#include <memory>

#include <Silica/include/SWidget.h>

#include "AxionEngine/Source/scene/Entity.h"

namespace Silica {
	class SBox;
	class SVerticalBox;
}

namespace Axion {

	class EntityPropertiesPanel {
	public:

		EntityPropertiesPanel() = default;
		~EntityPropertiesPanel() = default;

		Silica::WidgetPtr getWidget();

		void setEntity(Entity entity);
		void setHierarchyRefreshCallback(std::function<void()> callback);

	private:

		void rebuildUI();

		Entity m_selectedEntity;
		std::function<void()> m_onHierarchyNeedsRefresh;

		std::shared_ptr<Silica::SBox> m_uiRoot;
		std::shared_ptr<Silica::SVerticalBox> m_contentBox;

	};

}
