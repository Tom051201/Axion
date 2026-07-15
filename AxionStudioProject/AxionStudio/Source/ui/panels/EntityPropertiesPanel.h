#pragma once

#include "AxionStudio/Vendor/Silica/include/SBox.h"
#include "AxionStudio/Vendor/Silica/include/SVerticalBox.h"

#include "AxionEngine/Source/scene/Entity.h"

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
