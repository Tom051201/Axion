#pragma once

#include <string>
#include <optional>
#include <functional>

#include "SWidget.h"
#include "FontAtlas.h"

namespace Silica {

	class SEditableText : public SWidget {
	public:

		struct Args {
			std::string initialText = "";
			std::string hintText = "Type here...";
			std::optional<Color> textColor;
			std::optional<Color> backgroundColor;
			std::optional<Color> focusedColor;
			FontAtlas* font = nullptr;

			std::function<void(const std::string&)> onTextChanged = nullptr;
			std::function<void(const std::string&)> onTextCommitted = nullptr;
		};

		void construct(const Args& args);

		void computeDesiredSize() override;
		void arrangeChildren(const Geometry& allocatedGeometry) override;
		void onDraw(DrawList& outDrawList, const Geometry& allocatedGeometry) const override;

		EventReply onMouseMove(const Geometry& allocatedGeometry, const Vec2& mousePos) override;
		EventReply onMouseButtonDown(const Geometry& allocatedGeometry, const Vec2& mousePos, MouseButton button) override;
		EventReply onChar(char c) override;
		EventReply onKeyDown(Key key) override;

	private:

		std::string m_text;
		std::string m_hintText;
		Color m_textColor;
		Color m_backgroundColor;
		Color m_focusedColor;
		FontAtlas* m_font;

		int m_cursorIndex = 0;

		std::function<void(const std::string&)> m_onTextChanged;
		std::function<void(const std::string&)> m_onTextCommitted;

		void addRectToDrawList(DrawList& drawList, const Geometry& geo, Color color) const;

	};

}
