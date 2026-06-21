#pragma once

#include "AxionStudio/Vendor/Silica/include/SWidget.h"
#include "AxionStudio/Vendor/Silica/include/SBox.h"
#include "AxionStudio/Vendor/Silica/include/SScrollBox.h"
#include "AxionStudio/Vendor/Silica/include/SVerticalBox.h"
#include "AxionStudio/Vendor/Silica/include/FontAtlas.h"

#include "AxionEngine/Source/scene/Entity.h"

#include <functional>

namespace Axion {

	class EntityPropertiesPanel {
	public:

		EntityPropertiesPanel() = default;
		~EntityPropertiesPanel() = default;

		Silica::WidgetPtr getWidget(Silica::FontAtlas* font);

		void setEntity(Entity entity);

		void setHierarchyRefreshCallback(std::function<void()> callback);

	private:

		void rebuildUI();

		Silica::FontAtlas* m_font = nullptr;
		Entity m_selectedEntity;
		std::function<void()> m_onHierarchyNeedsRefresh;

		std::shared_ptr<Silica::SScrollBox> m_uiRoot;
		std::shared_ptr<Silica::SVerticalBox> m_contentBox;

	};

}
