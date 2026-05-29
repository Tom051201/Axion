#pragma once

#include <functional>
#include <optional>

#include "SWidget.h"

namespace Silica {

	class SCheckBox : public SWidget {
	public:

		struct Args {
			bool initialCheck = false;
			std::optional<Color> backgroundColor;
			std::optional<Color> checkColor;
			std::function<void(bool)> onCheckChanged = nullptr;
		};

		void construct(const Args& args);

		void computeDesiredSize() override;
		void arrangeChildren(const Geometry& allocatedGeometry) override;
		void onDraw(DrawList& outDrawList, const Geometry& allocatedGeometry) const override;

		EventReply onMouseButtonDown(const Geometry& allocatedGeometry, const Vec2& mousePos, MouseButton button) override;

	private:

		bool m_isChecked = false;
		Color m_backgroundColor;
		Color m_checkColor;
		std::function<void(bool)> m_onCheckChanged;

		void addRectToDrawList(DrawList& drawList, const Geometry& geo, Color color) const;

	};

}