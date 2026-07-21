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
			std::function<void()> onClicked = nullptr;
			std::function<void()> onDragStart = nullptr;
			std::function<EventReply(const DragDropPayload&)> onDragOver = nullptr;
			std::function<EventReply(const DragDropPayload&)> onDrop = nullptr;
			std::function<void(bool)> onToggleOpen = nullptr;
			std::vector<WidgetPtr> children;
		};

		void construct(const Args& args);

		void computeDesiredSize() override;
		void arrangeChildren(const Geometry& allocatedGeometry) override;
		void onDraw(DrawList& outDrawList, const Geometry& allocatedGeometry) const override;

		void setRenderScale(float scale) override;

		EventReply onMouseMove(const Geometry& allocatedGeometry, const Vec2& mousePos) override;
		EventReply onMouseButtonDown(const Geometry& allocatedGeometry, const Vec2& mousePos, MouseButton button) override;
		EventReply onMouseButtonUp(const Geometry& allocatedGeometry, const Vec2& mousePos, MouseButton button) override;
		EventReply onDragOver(const Geometry& allocatedGeometry, const Vec2& mousePos, const DragDropPayload& payload) override;
		EventReply onDrop(const Geometry& allocatedGeometry, const Vec2& mousePos, const DragDropPayload& payload) override;

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
		std::function<void(bool)> m_onToggleOpen;

		std::vector<WidgetPtr> m_children;

		float m_headerHeight = 22.0f;
		float m_indentSize = 15.0f;

		std::function<void()> m_onDragStart;
		std::function<EventReply(const DragDropPayload&)> m_onDragOver;
		std::function<EventReply(const DragDropPayload&)> m_onDrop;
		bool m_isLeftMouseDown = false;

		void drawTriangle(DrawList& drawList, const Vec2& center, float radius, Color color, bool pointDown) const;

	};

}
