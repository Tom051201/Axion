#pragma once
#include <string>
#include <functional>

#include "SWidget.h"
#include "MathTypes.h"
#include "FontAtlas.h"

namespace Silica {

	class SVector3Input : public SWidget {
	public:

		struct Args {
			std::string label;
			Vec3 initialValue;
			FontAtlas* font = nullptr;
			float labelWidth = 70.0f;
			Color firstColor = Color(200, 50, 50, 255);
			Color secondColor = Color(50, 200, 50, 255);
			Color thirdColor = Color(50, 50, 200, 255);
			std::string firstText = "X";
			std::string secondText = "Y";
			std::string thirdText = "Z";
			std::function<void(Vec3)> onValueChanged = nullptr;
		};

		void construct(const Args& args);

		void computeDesiredSize() override;
		void arrangeChildren(const Geometry& allocatedGeometry) override;
		void onDraw(DrawList& outDrawList, const Geometry& allocatedGeometry) const override;

		EventReply onMouseMove(const Geometry& allocatedGeometry, const Vec2& mousePos) override;
		EventReply onMouseButtonDown(const Geometry& allocatedGeometry, const Vec2& mousePos, MouseButton button) override;
		EventReply onMouseButtonUp(const Geometry& allocatedGeometry, const Vec2& mousePos, MouseButton button) override;

	private:

		WidgetPtr m_rootAssembly;
		Vec3 m_currentValue;

	};

}
