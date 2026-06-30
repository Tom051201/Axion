#pragma once

#include <optional>

#include "SWidget.h"
#include "Renderer.h"

namespace Silica {

	class SImage : public SWidget {
	public:

		struct Args {
			TextureID textureID = 0;
			std::optional<Color> tint;
			Vec2 desiredSize = Vec2(100.0f, 100.0f);
		};

		void construct(const Args& args);

		void computeDesiredSize() override {}
		void arrangeChildren(const Geometry& allocatedGeometry) override;
		void onDraw(DrawList& outDrawList, const Geometry& allocatedGeometry) const override;

		void setTextureID(TextureID id);
		void setDesiredSize(const Vec2& size);

	private:

		TextureID m_textureId;
		Color m_tintColor;

	};

}
