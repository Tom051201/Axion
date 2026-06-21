#pragma once

#include "AxionStudio/Vendor/Silica/include/SWidget.h"
#include "AxionStudio/Vendor/Silica/include/SBox.h"
#include "AxionStudio/Vendor/Silica/include/SScrollBox.h"
#include "AxionStudio/Vendor/Silica/include/SVerticalBox.h"
#include "AxionStudio/Vendor/Silica/include/FontAtlas.h"

#include "AxionEngine/Source/scene/Scene.h"
#include "AxionEngine/Source/scene/Entity.h"

#include <functional>

namespace Axion {

	class HierarchyPanel {
	public:

		HierarchyPanel() = default;
		~HierarchyPanel() = default;

		Silica::WidgetPtr getWidget(Silica::FontAtlas* font);

		void setScene(Ref<Scene> scene);
		void refresh();

		void setSelectionCallback(std::function<void(Entity)> callback);

	private:

		void rebuildUI();
		Silica::WidgetPtr buildEntityNode(Entity entity);

		Silica::FontAtlas* m_font = nullptr;
		Ref<Scene> m_scene;
		std::function<void(Entity)> m_onEntitySelected;

		std::shared_ptr<Silica::SBox> m_uiRoot;
		std::shared_ptr<Silica::SVerticalBox> m_contentBox;

	};

}
