#pragma once

#include "MathTypes.h"

namespace Silica {

	struct Theme {

		// -- TEXT --
		Color textMain = Color::white();
		Color textDim = Color(150, 150, 150);

		// -- BACKGROUNDS --
		Color backgroundWindow = Color(40, 40, 40);
		Color backgroundPanel = Color(37, 37, 38);
		Color backgroundDarkWorkspace = Color(17, 17, 17);

		// -- BUTTONS --
		Color buttonNormal = Color(58, 58, 58);
		Color buttonHover = Color(80, 80, 80);
		Color buttonPressed = Color(42, 42, 42);

		// -- ACCENTS --
		Color accentPrimary = Color(70, 130, 180);

	};

	Theme& GetTheme();

}
