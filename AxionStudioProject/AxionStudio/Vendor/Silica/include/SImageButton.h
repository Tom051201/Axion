#pragma once

#include <optional>
#include <functional>

#include "SWidget.h"
#include "Renderer.h"

namespace Silica {

	class SImageButton : public SWidget {
	public:

		struct Args {
			TextureID textureID = 0;
			Vec2 desiredSize = Vec2(32.0f, 32.0f);
			std::optional<Color> normalTint;
			std::optional<Color> hoverTint;
			std::optional<Color> pressedTint;
			std::function<EventReply()> onClick = nullptr;
		};

		void construct(const Args& args);

		void computeDesiredSize() override {}
		void arrangeChildren(const Geometry& allocatedGeometry) override;
		void onDraw(DrawList& outDrawList, const Geometry& allocatedGeometry) const override;

		EventReply onMouseMove(const Geometry& allocatedGeometry, const Vec2& mousePos) override;
		EventReply onMouseButtonDown(const Geometry& allocatedGeometry, const Vec2& mousePos, MouseButton button) override;
		EventReply onMouseButtonUp(const Geometry& allocatedGeometry, const Vec2& mousePos, MouseButton button) override;

	private:

		TextureID m_textureID = 0;
		Color m_normalTint;
		Color m_hoverTint;
		Color m_pressedTint;
		std::function<EventReply()> m_onClick;

		bool m_isPressed = false;

	};

}
