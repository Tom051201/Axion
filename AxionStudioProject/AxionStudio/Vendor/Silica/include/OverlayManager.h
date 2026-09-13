#pragma once

#include <vector>
#include <functional>
#include <cstdint>

#include "SWidget.h"

namespace Silica {

	enum class OverlayType : uint8_t {
		Popup,
		Modal,
		Tooltip
	};

	using OverlayID = uint32_t;

	struct Overlay {
		OverlayID id = 0;
		OverlayID parentID = 0;

		WidgetPtr widget;
		Geometry geometry;

		OverlayType type = OverlayType::Popup;

		bool blocksInputBelow = true;
		bool dismissOnOutsideClick = true;
		bool closeOnEscape = true;

		std::function<void()> closeCallback;
	};



	class OverlayManager {
	public:

		OverlayID open(WidgetPtr widget,const Geometry& geometry, OverlayType type = OverlayType::Popup, OverlayID parentID = 0, bool blocksInputBelow = true, bool dismissOnOutsideClick = true, bool closeOnEscape = true, std::function<void()> closeCallback = nullptr);

		void update(OverlayID id, const Geometry& geometry);

		void close(OverlayID id);
		void closeChildren(OverlayID parentID);
		void closeAll();
		void closePopups();

		const std::vector<Overlay>& getOverlays() const {
			return m_overlays;
		}

		std::vector<Overlay>& getOverlays() {
			return m_overlays;
		}

		bool isOpen(OverlayID id) const;
		bool contains(OverlayID id, const Vec2& mousePos) const;
		bool blocksInputAt(const Vec2& mousePos) const;

	private:

		std::vector<Overlay> m_overlays;
		OverlayID m_nextID = 1;

	};

}
