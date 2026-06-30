#pragma once
#include <string>
#include <vector>

#include "SWidget.h"
#include "FontAtlas.h"
#include "Renderer.h"

namespace Silica {

	class STreeNode : public SWidget {
	public:

		struct Args {
			std::string label;
			FontAtlas* font = nullptr;
			float yTextOffset = 16.0f;
			bool initiallyOpen = false;
			bool isSelected = false;
			std::function<bool()> isDragged = nullptr;
			std::vector<WidgetPtr> children;
			std::function<void()> onClicked = nullptr;
			std::function<void()> onDragStart = nullptr;
			std::function<EventReply()> onDrop = nullptr;
		};

		void construct(const Args& args);

		void computeDesiredSize() override;
		void arrangeChildren(const Geometry& allocatedGeometry) override;
		void onDraw(DrawList& outDrawList, const Geometry& allocatedGeometry) const override;

		EventReply onMouseMove(const Geometry& allocatedGeometry, const Vec2& mousePos) override;
		EventReply onMouseButtonDown(const Geometry& allocatedGeometry, const Vec2& mousePos, MouseButton button) override;
		EventReply onMouseButtonUp(const Geometry& allocatedGeometry, const Vec2& mousePos, MouseButton button) override;

		void addChild(WidgetPtr child);
		void clearChildren();

		bool isOpen() const;
		void setOpen(bool open);
		void setSelected(bool selected);

	private:

		std::string m_label;
		FontAtlas* m_font = nullptr;
		float m_yOffset = 16.0f;
		bool m_isOpen = false;
		bool m_isSelected = false;
		bool m_isHovered = false;
		std::function<bool()> m_isDragged;
		std::function<void()> m_onClicked;

		std::vector<WidgetPtr> m_children;

		float m_headerHeight = 22.0f;
		float m_indentSize = 15.0f;

		std::function<void()> m_onDragStart;
		std::function<EventReply()> m_onDrop;
		bool m_isLeftMouseDown = false;

		void addRectToDrawList(DrawList& drawList, const Geometry& geo, Color color) const;
		void drawTriangle(DrawList& drawList, const Vec2& center, float radius, Color color, bool pointDown) const;

	};

}
