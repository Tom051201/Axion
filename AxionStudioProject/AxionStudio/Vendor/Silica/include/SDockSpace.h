#pragma once

#include <memory>
#include <optional>
#include <vector>
#include <string>
#include <filesystem>
#include <fstream>
#include <unordered_map>

#include "SWidget.h"
#include "FontAtlas.h"

namespace Silica {

	struct SDockNode;
	using DockNodePtr = std::shared_ptr<SDockNode>;



	enum class SplitDirection { None, Horizontal, Vertical };
	enum class DockZone { None, Left, Right, Top, Bottom, Center };



	struct DockTab {
		std::string title;
		WidgetPtr content;
		Rect hitRect;
		Rect closeRect;
	};



	struct SDockNode {
		SplitDirection splitDirection = SplitDirection::None;
		float splitRatio = 0.5f;

		DockNodePtr child[2];

		std::vector<DockTab> tabs;
		int activeTab = 0;

		Geometry allocatedGeometry;
		Rect splitterRect;
		Rect titleBarRect;
	};



	class SDockSpace : public SWidget {
	public:

		struct Args {
			std::vector<DockTab> initialTabs;
			std::optional<Color> titleBarColor;
			FontAtlas* font = nullptr;
			std::function<void(std::string, WidgetPtr, Vec2)> onUndockWindow = nullptr;
		};

		void construct(const Args& args);

		void computeDesiredSize() override;
		void arrangeChildren(const Geometry& allocatedGeometry) override;
		void onDraw(DrawList& outDrawList, const Geometry& allocatedGeometry) const override;

		EventReply onMouseMove(const Geometry& allocatedGeometry, const Vec2& mousePos) override;
		EventReply onMouseButtonDown(const Geometry& allocatedGeometry, const Vec2& mousePos, MouseButton button) override;
		EventReply onMouseButtonUp(const Geometry& allocatedGeometry, const Vec2& mousePos, MouseButton button) override;
		EventReply onMouseWheel(const Geometry& allocatedGeometry, const Vec2& mousePos, float scrollDelta) override;

		void splitNode(DockNodePtr node, SplitDirection dir, float ratio, std::string title, WidgetPtr newContent, bool insertFirst = false);
		DockNodePtr getRootNode() const { return m_rootNode; }
		void updateDragDropPreview(const Vec2& mousePos, bool isDragging);
		bool processDrop(std::string title, WidgetPtr draggedContent);

		void saveLayout(const std::filesystem::path& filePath);
		void loadLayout(const std::filesystem::path& filePath);

		void registerTab(const std::string& title, WidgetPtr content);
		void openTab(const std::string& title);
		void closeTab(DockNodePtr node, int tabIndex);

		std::vector<std::string> getRegisteredTabNames() const;

	private:

		DockNodePtr m_rootNode;

		// -- Splitter dragging --
		DockNodePtr m_draggingNode;
		DockNodePtr m_hoveredNode = nullptr;

		// -- Tab clicking/dragging --
		DockNodePtr m_pressedTabNode = nullptr;
		int m_pressedTabIndex = -1;
		Vec2 m_pressedMousePos;

		DockNodePtr m_previewNode = nullptr;
		float m_splitterThickness = 4.0f;
		DockZone m_previewZone = DockZone::None;

		std::function<void(std::string, WidgetPtr, Vec2)> m_onUndockWindow;
		Color m_titleBarColor;
		FontAtlas* m_font = nullptr;

		std::unordered_map<std::string, WidgetPtr> m_widgetRegistry;

		void arrangeNode(DockNodePtr node, const Geometry& geo);
		void drawNode(const DockNodePtr& node, DrawList& drawList) const;
		void addRectToDrawList(DrawList& drawList, const Geometry& geo, Color color) const;

		DockNodePtr hitTestSplitter(const DockNodePtr& node, const Vec2& mousePos);
		DockNodePtr hitTestContentNode(const DockNodePtr& node, const Vec2& mousePos);
		std::pair<DockNodePtr, int> hitTestTab(const DockNodePtr& node, const Vec2& mousePos);

		bool removeLeafNode(DockNodePtr parent, DockNodePtr target);
		void undockNode(DockNodePtr node, int tabIndex, const Vec2& mousePos);

		// -- Serialzing --
		void serializeNode(std::ofstream& out, DockNodePtr node, int depth);
		DockNodePtr deserializeNode(std::ifstream& in);
		static std::string trim(const std::string& str);
	};

}
