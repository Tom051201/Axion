#pragma once

#include <string>
#include <optional>

#include "SWidget.h"
#include "FontAtlas.h"

namespace Silica {

	class SWindow : public SWidget {
	public:

		struct Args {
			std::string title = "Window";
			Vec2 initialPosition = { 100.0f, 100.0f };
			Vec2 initialSize = { 400.0f, 300.0f };
			std::optional<Color> backgroundColor;
			std::optional<Color> titleBarColor;
			std::optional<Color> titleBarDraggingColor;
			std::optional<Color> titleTextColor;
			FontAtlas* font = nullptr;
			WidgetPtr content = nullptr;
		};

		void construct(const Args& args);

		void computeDesiredSize() override;
		void arrangeChildren(const Geometry& allocatedGeometry) override;
		void onDraw(DrawList& outDrawList, const Geometry& allocatedGeometry) const override;

		EventReply onMouseMove(const Geometry& allocatedGeometry, const Vec2& mousePos) override;
		EventReply onMouseButtonDown(const Geometry& allocatedGeometry, const Vec2& mousePos, MouseButton button) override;
		EventReply onMouseButtonUp(const Geometry& allocatedGeometry, const Vec2& mousePos, MouseButton button) override;

		bool isDragging() const;
		void startDragging(const Vec2& mousePos);
		void setContent(WidgetPtr content);
		WidgetPtr getContent() const;
		const std::string& getTitle() const;

		std::function<void(Vec2)> onDragMove = nullptr;
		std::function<void(Vec2)> onDragEnd = nullptr;

	private:

		std::string m_title;
		Vec2 m_position;
		Vec2 m_size;
		WidgetPtr m_content;
		FontAtlas* m_font;

		Color m_backgroundColor;
		Color m_titleBarColor;
		Color m_titleBarDraggingColor;
		Color m_titleTextColor;

		bool m_isDragging = false;
		Vec2 m_dragClickOffset;

		Rect getTitleBarRect() const;
		void addRectToDrawList(DrawList& drawList, const Geometry& geo, Color color) const;

	};

}
