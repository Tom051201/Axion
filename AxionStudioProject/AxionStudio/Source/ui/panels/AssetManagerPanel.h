#pragma once

#include <memory>
#include <string>
#include <unordered_set>

#include <Silica/include/SWidget.h>

#include "AxionEngine/Source/events/ApplicationEvent.h"

namespace Silica {
	class SBox;
}

namespace Axion {

	class AssetManagerPanel {
	public:

		AssetManagerPanel() = default;
		~AssetManagerPanel() = default;

		Silica::WidgetPtr getWidget();

		void onEvent(Event& ev);

		void refresh();

		bool isExpanded(const std::string& key) const;
		void setExpanded(const std::string& key, bool expanded);
		bool checkAndRegisterExpanded(const std::string& key);
		void expandAll();
		void collapseAll();

	private:

		std::shared_ptr<Silica::SBox> m_uiRoot;
		bool m_rebuildQueued = false;

		std::unordered_set<std::string> m_expandedCategories;
		bool m_forceExpandAll = false;

		void rebuildUI_Internal();

		EventReply onSceneChanged(SceneChangedEvent& ev);

	};

}
