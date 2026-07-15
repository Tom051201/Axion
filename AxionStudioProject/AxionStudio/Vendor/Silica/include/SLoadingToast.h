#pragma once

#include <string>
#include <functional>

#include "SWidget.h"
#include "FontAtlas.h"

namespace Silica {

	class SLoadingToast : public SWidget {
	public:

		struct Args {
			std::string text = "Loading...";
			FontAtlas* font = nullptr;
			std::function<bool()> isVisible = nullptr;
		};

		void construct(const Args& args);

		void computeDesiredSize() override;
		void onDraw(DrawList& outDrawList, const Geometry& allocatedGeometry) const override;

	private:

		std::string m_text;
		FontAtlas* m_font = nullptr;
		std::function<bool()> m_isVisible;

	};

}