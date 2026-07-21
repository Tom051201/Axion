#pragma once

#include <string>
#include <functional>

#include "SWidget.h"
#include "MathTypes.h"
#include "FontAtlas.h"

namespace Silica {

	class SInputFieldFloat;

	class SInputFieldVec3Float : public SWidget {
	public:

		struct Args {
			std::string label;
			Vec3 initialValue;
			Vec3 resetValue = Vec3(0.0f, 0.0f, 0.0f);
			float labelWidth = 70.0f;
			Color firstColor = Color(200, 50, 50, 255);
			Color secondColor = Color(50, 200, 50, 255);
			Color thirdColor = Color(50, 50, 200, 255);
			std::string firstText = "X";
			std::string secondText = "Y";
			std::string thirdText = "Z";
			FontAtlas* font = nullptr;
			std::function<void(Vec3)> onValueChanged = nullptr;
		};

		void construct(const Args& args);

		void computeDesiredSize() override;
		void arrangeChildren(const Geometry& allocatedGeometry) override;
		void onDraw(DrawList& outDrawList, const Geometry& allocatedGeometry) const override;

		void setRenderScale(float scale) override;

		void setValue(const Vec3& newValue);

		EventReply onMouseMove(const Geometry& allocatedGeometry, const Vec2& mousePos) override;
		EventReply onMouseButtonDown(const Geometry& allocatedGeometry, const Vec2& mousePos, MouseButton button) override;
		EventReply onMouseButtonUp(const Geometry& allocatedGeometry, const Vec2& mousePos, MouseButton button) override;
		EventReply onMouseWheel(const Geometry& allocatedGeometry, const Vec2& mousePos, float scrollDelta) override;
		EventReply onDragOver(const Geometry& allocatedGeometry, const Vec2& mousePos, const DragDropPayload& payload) override;
		EventReply onDrop(const Geometry& allocatedGeometry, const Vec2& mousePos, const DragDropPayload& payload) override;

	private:

		WidgetPtr m_rootAssembly;
		Vec3 m_currentValue;
		Vec3 m_resetValue;

		std::shared_ptr<SInputFieldFloat> m_inputX;
		std::shared_ptr<SInputFieldFloat> m_inputY;
		std::shared_ptr<SInputFieldFloat> m_inputZ;

	};

}
