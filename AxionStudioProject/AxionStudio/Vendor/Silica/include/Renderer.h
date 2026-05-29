#pragma once

#include <stdint.h>
#include <vector>
#include <functional>

#include "MathTypes.h"
#include "SWidget.h"

namespace Silica {

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

		void addThickLine(const Vec2& p0, const Vec2& p1, float thickness, Color color);
		void addBezierCurve(const Vec2& p0, const Vec2& p1, const Vec2& p2, const Vec2& p3, float thickness, Color color);

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

}



// ----- UI Renderer -----
#include "SWidget.h"

namespace Silica {

	class Renderer {
	public:

		static DrawList s_drawList;
		static Vec2 s_mousePosition;

		static void render(WidgetPtr rootWidget, float screenWidth, float screenHeight);

		static void processMouseMove(WidgetPtr rootWidget, float screenWidth, float screenHeight, float mouseX, float mouseY);
		static void processMouseDown(WidgetPtr rootWidget, float screenWidth, float screenHeight, float mouseX, float mouseY, MouseButton button);
		static void processMouseUp(WidgetPtr rootWidget, float screenWidth, float screenHeight, float mouseX, float mouseY, MouseButton button);
		static void processMouseWheel(WidgetPtr rootWidget, float screenWidth, float screenHeight, float mouseX, float mouseY, float scrollDelta);

		static const DrawList* getDrawData() { return &s_drawList; }

		static void pushPopup(WidgetPtr widget, const Geometry& geo, std::function<void()> closeCallback);

	private:

		static std::vector<PopupRecord> s_popups;

	};

}
