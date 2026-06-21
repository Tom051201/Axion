#pragma once

#include <string>
#include <functional>
#include <filesystem>
#include <fstream>

#include "MathTypes.h"
#include "FontAtlas.h"
#include "SWidget.h"

namespace Silica {

	using PinID = int;
	using NodeID = int;
	using LinkID = int;

	enum class PinType { Input, Output };

	struct NodePin {
		PinID id;
		std::string name;
		PinType type;
		Color color = Color(200, 200, 200);
		Vec2 screenPosition;
	};

	struct GraphNode {
		NodeID id;
		std::string title;
		Color headerColor = Color(40, 100, 150, 255);
		Vec2 position = { 0.0f, 0.0f };
		Vec2 size = { 150, 100 };
		std::vector<NodePin> inputs;
		std::vector<NodePin> outputs;
	};

	struct GraphLink {
		LinkID id;
		PinID startPin;
		PinID endPin;
		Color color = Color::white();
	};

	class SNodeEditor : public SWidget {
	public:

		struct Args {
			FontAtlas* font = nullptr;
			std::function<WidgetPtr(Vec2)> onBackgroundContextClick;
			std::function<WidgetPtr(NodeID, Vec2)> onNodeContextClick;
		};

		void construct(const Args& args);

		void computeDesiredSize() override;
		void arrangeChildren(const Geometry& allocatedGeometry) override;
		void onDraw(DrawList& outDrawList, const Geometry& allocatedGeometry) const override;

		EventReply onMouseMove(const Geometry& allocatedGeometry, const Vec2& mousePos) override;
		EventReply onMouseButtonDown(const Geometry& allocatedGeometry, const Vec2& mousePos, MouseButton button) override;
		EventReply onMouseButtonUp(const Geometry& allocatedGeometry, const Vec2& mousePos, MouseButton button) override;
		EventReply onMouseWheel(const Geometry& allocatedGeometry, const Vec2& mousePos, float scrollDelta) override;
		EventReply onKeyDown(Key key) override;

		void addNode(const GraphNode& node);
		void addLink(LinkID id, PinID startPin, PinID endPin, Color color = Color::white());

		const std::vector<GraphNode>& getNodes() const;
		const std::vector<GraphLink>& getLinks() const;

		void clear();

		void saveGraph(const std::filesystem::path& filepath);
		void loadGraph(const std::filesystem::path& filepath);

	private:

		FontAtlas* m_font = nullptr;
		std::function<WidgetPtr(Vec2)> m_onBackgroundContextClick;
		std::function<WidgetPtr(NodeID, Vec2)> m_onNodeContextClick;

		WidgetPtr m_activeContextMenu = nullptr;
		Geometry m_contextMenuGeometry;

		std::vector<GraphNode> m_nodes;
		std::vector<GraphLink> m_links;
		int m_nextPinID = 1000;

		Vec2 m_rightClickStartPos;
		Vec2 m_panOffset = { 0.0f, 0.0f };
		float m_zoom = 1.0f;
		bool m_isPanning = false;
		Vec2 m_lastMousePos;

		NodeID m_draggingNodeID = -1;
		Vec2 m_nodeDragOffset;

		PinID m_draggingPinID = -1;
		Vec2 m_dragWireEndPos;

		NodeID m_selectedNodeID = -1;
		LinkID m_selectedLinkID = -1;

		void addRectToDrawList(DrawList& drawList, const Geometry& geo, Color color, float rounding = 0.0f) const;
		void drawText(DrawList& drawList, const std::string& text, Vec2 pos, Color color) const;
		GraphNode* findNode(NodeID id);
		NodePin* findPin(PinID id, GraphNode** outNode = nullptr);
		PinID hitTestPins(const Vec2& mousePos);
		NodeID hitTestNodes(const Vec2& mousePos);
		LinkID hitTestLinks(const Vec2& mousePos);

	};

}
