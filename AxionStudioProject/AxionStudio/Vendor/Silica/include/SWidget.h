#pragma once

#include <memory>
#include <vector>
#include <functional>

#include "Geometry.h"
#include "InputCodes.h"

namespace Silica {

	namespace Platform {
		enum class Cursor { Arrow, TextInput, ResizeEW, ResizeNS };
		extern void setCursor(Cursor cursor);
		extern void setMouseCapture(bool capture);
	}

	struct DrawList;



	// ----- SWidget Base Class -----
	class SWidget;
	using WidgetPtr = std::shared_ptr<SWidget>;

	class SWidget {
	public:

		virtual ~SWidget() = default;

		virtual void computeDesiredSize() = 0;
		virtual void arrangeChildren(const Geometry& allocatedGeometry) {
			m_allocatedGeometry = allocatedGeometry;
		}
		virtual void onDraw(DrawList& outDrawList, const Geometry& allocatedGeometry) const = 0;

		virtual EventReply onMouseMove(const Geometry& allocatedGeometry, const Vec2& mousePos) { return EventReply::unhandled(); }
		virtual EventReply onMouseButtonDown(const Geometry& allocatedGeometry, const Vec2& mousePos, MouseButton button) { return EventReply::unhandled(); }
		virtual EventReply onMouseButtonUp(const Geometry& allocatedGeometry, const Vec2& mousePos, MouseButton button) { return EventReply::unhandled(); }
		virtual EventReply onChar(char c) { return EventReply::unhandled(); }
		virtual EventReply onKeyDown(Key key) { return EventReply::unhandled(); }
		virtual EventReply onKeyUp(Key key) { return EventReply::unhandled(); }
		virtual EventReply onMouseWheel(const Geometry& allocatedGeometry, const Vec2& mousePos, float scrollDelta) { return EventReply::unhandled(); }

		static void setFocusedWidget(SWidget* widget) { s_focusedWidget = widget; }
		static SWidget* getFocusedWidget() { return s_focusedWidget; }

		static void setCapturedWidget(SWidget* widget) {
			s_capturedWidget = widget;
			Platform::setMouseCapture(widget != nullptr);
		}
		static SWidget* getCapturedWidget() { return s_capturedWidget; }

		Vec2 getDesiredSize() const { return m_desiredSize; }
		const Geometry& getAllocatedGeometry() const { return m_allocatedGeometry; }

	protected:

		static SWidget* s_focusedWidget;
		static SWidget* s_capturedWidget;

		Vec2 m_desiredSize = Vec2::zero();
		Geometry m_allocatedGeometry;
		bool m_isHovered = false;

	};



	// ----- Make Widget Factory -----
	template<typename T>
	std::shared_ptr<T> MakeWidget(const typename T::Args& args) {
		std::shared_ptr<T> widget = std::make_shared<T>();
		widget->construct(args);
		return widget;
	}



	// ----- Slots -----
	struct Slot {
		Vec2 padding = Vec2::zero();
		WidgetPtr child = nullptr;
	};


}
