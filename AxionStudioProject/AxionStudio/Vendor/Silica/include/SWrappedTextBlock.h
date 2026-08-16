#pragma once

#include <string>
#include <optional>

#include "SWidget.h"
#include "FontAtlas.h"

namespace Silica {

	class SWrappedTextBlock : public SWidget {
	public:

		struct Args {
			std::string text;
			float wrapWidth;
			int maxLines = 0;
			std::optional<Color> color;
			FontAtlas* font = nullptr;
		};

		void construct(const Args& args);

		void computeDesiredSize() override;
		void arrangeChildren(const Geometry& allocatedGeometry) override;
		void onDraw(DrawList& outDrawList, const Geometry& allocatedGeometry) const override;

		void setText(const std::string& text);

	private:

		std::string m_text;
		std::string m_wrappedText;
		float m_wrapWidth = 0.0f;
		int m_maxLines = 0;
		Color m_color;
		FontAtlas* m_font = nullptr;

		void updateWrappedText();

	};

}
