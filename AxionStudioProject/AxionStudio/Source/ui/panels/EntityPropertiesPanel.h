#pragma once

#include <functional>
#include <memory>

#include <Silica/include/SWidget.h>

#include "AxionEngine/Source/scene/Entity.h"

#include "AxionStudio/Source/core/EditorEvents.h"

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

		void onEvent(Event& e);

		void setEntity(Entity entity);

	private:

		void rebuildUI();

		Entity m_selectedEntity;

		std::shared_ptr<Silica::SBox> m_uiRoot;
		std::shared_ptr<Silica::SVerticalBox> m_contentBox;

		EventReply onEntitySelected(EntitySelectedEvent& e);

	};

}
