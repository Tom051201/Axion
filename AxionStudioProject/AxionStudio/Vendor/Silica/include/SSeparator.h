#pragma once

#include <optional>

#include "SWidget.h"

namespace Silica {

	enum class Orientation {
		Horizontal,
		Vertical
	};

	class SSeparator : public SWidget {
	public:

		struct Args {
			Orientation orientation = Orientation::Horizontal;
			float thickness = 4.0f;
			std::optional<Color> color;
		};

		void construct(const Args& args);

		void computeDesiredSize() override;
		void arrangeChildren(const Geometry& allocatedGeometry) override;
		void onDraw(DrawList& outDrawList, const Geometry& allocatedGeometry) const override;

	private:

		Orientation m_orientation = Orientation::Horizontal;
		float m_thickness = 2.0f;
		Color m_color;

	};

}
