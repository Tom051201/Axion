#pragma once

#include <string>
#include <optional>

#include "SWidget.h"
#include "FontAtlas.h"
#include "MathTypes.h"

namespace Silica {

	class STextBlock : public SWidget {
	public:

		struct Args {
			std::string text = "";
			std::optional<Color> color;
			FontAtlas* font = nullptr;
		};

		void construct(const Args& args);

		void computeDesiredSize() override;
		void arrangeChildren(const Geometry& allocatedGeometry) override;
		void onDraw(DrawList& outDrawList, const Geometry& allocatedGeometry) const override;

	private:

		std::string m_text;
		Color m_color;
		FontAtlas* m_font;

	};

}
