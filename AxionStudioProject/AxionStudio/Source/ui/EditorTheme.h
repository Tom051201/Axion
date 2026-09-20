#pragma once

#include <Silica/include/MathTypes.h>
#include <Silica/include/Theme.h>

namespace Axion::EditorTheme {

	// ----- Toolbars -----
	constexpr float TOOLBAR_HEIGHT = 36.0f;
	constexpr float TOOLBAR_PADDING_X = 6.0f;
	constexpr float TOOLBAR_SPACING = 5.0f;

	// ----- Buttons -----
	constexpr float BUTTON_PADDING_X = 8.0f;
	constexpr float BUTTON_PADDING_Y = 4.0f;
	constexpr Silica::Color BUTTON_COLOR_HOVER_SUBTLE = Silica::Color(255, 255, 255, 20);

	// ----- Icons -----
	constexpr float ICON_SIZE_SMALL = 16.0f;
	constexpr float ICON_SIZE_MEDIUM = 24.0f;
	constexpr float ICON_SIZE_LARGE = 32.0f;

	// ----- Standard Padding and Spacing -----
	constexpr float PADDING_SMALL = 4.0f;
	constexpr float PADDING_MEDIUM = 8.0f;
	constexpr float PADDING_LARGE = 15.0f;
	constexpr float PADDING_XLARGE = 20.0f;

	constexpr float SPACING_SMALL = 2.0f;
	constexpr float SPACING_MEDIUM = 5.0f;
	constexpr float SPACING_LARGE = 15.0f;

	// ----- Tree Nodes -----
	constexpr float TREE_NODE_Y_OFFSET = 16.0f;

	// ----- Dropdowns and Menus -----
	constexpr float OPTIONS_MENU_WIDTH = 240.0f;

	// ----- Properties ------
	constexpr float ROW_LABEL_WIDTH = 120.0f;
	constexpr float ROW_HEIGHT_DEFAULT = 28.0f;

	// ----- Modals -----
	constexpr Silica::Color MODAL_COLOR_DIMMED_BACKGROUND = Silica::Color(0, 0, 0, 180);

}
