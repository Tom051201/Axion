#pragma once

#include <cstdint>
#include <vector>
#include <functional>
#include <string>

#include "MathTypes.h"
#include "SWidget.h"

namespace Silica {

	class FontAtlas;

	using TextureID = uint32_t;



	struct Vertex {
		Vec2 position;
		Vec2 uv;
		uint32_t color;
	};



	struct DrawCommand {
		uint32_t indexCount;
		uint32_t startIndex;
		int32_t vertexOffset;
		Rect clipRect;
		TextureID textureID;
	};



	struct DrawList {
		std::vector<Vertex> vertices;
		std::vector<uint32_t> indices;
		std::vector<DrawCommand> commands;
		std::vector<Rect> clipRectStack;
		std::vector<TextureID> textureIDStack;

		void addDrawCommand();

		void addRect(const Geometry& geo, Color color);
		void addGradientRect(const Geometry& geo, Color tl, Color tr, Color br, Color bl);
		void addThickLine(const Vec2& p0, const Vec2& p1, float thickness, Color color);
		void addBezierCurve(const Vec2& p0, const Vec2& p1, const Vec2& p2, const Vec2& p3, float thickness, Color color);
		void addText(FontAtlas* font, const std::string& text, Vec2 position, Color color, float scale = 1.0f, float lineHeight = 20.0f);

		Rect getCurrentClipRect() const;
		void pushClipRect(const Rect& rect);
		void popClipRect();

		TextureID getCurrentTextureID() const;
		void pushTextureID(TextureID id);
		void popTextureID();
	};

	struct PopupRecord {
		WidgetPtr widget;
		Geometry geometry;
		std::function<void()> closeCallback;
	};



	class Renderer {
	public:

		static void render(WidgetPtr rootWidget, float screenWidth, float screenHeight);

		static void processMouseMove(WidgetPtr rootWidget, float screenWidth, float screenHeight, float mouseX, float mouseY);
		static void processMouseDown(WidgetPtr rootWidget, float screenWidth, float screenHeight, float mouseX, float mouseY, MouseButton button);
		static void processMouseUp(WidgetPtr rootWidget, float screenWidth, float screenHeight, float mouseX, float mouseY, MouseButton button);
		static void processMouseWheel(WidgetPtr rootWidget, float screenWidth, float screenHeight, float mouseX, float mouseY, float scrollDelta);

		static const DrawList* getDrawData();
		static const Vec2& getMousePosition();

		static void pushPopup(WidgetPtr widget, const Geometry& geo, std::function<void()> closeCallback);
		static void closeAllPopups();

		static void setTooltip(const std::string& text, FontAtlas* font);

	private:

		static DrawList s_drawList;
		static Vec2 s_mousePosition;

		static std::vector<PopupRecord> s_popups;

		static std::string s_tooltipText;
		static FontAtlas* s_tooltipFont;

	};

}
