#pragma once

#include "MathTypes.h"
#include "FontAtlas.h"

namespace Silica {

	struct Theme {

		Color Accent_Primary = Color(70, 130, 180);
		Color Accent_Secondary = Color(100, 150, 200);
		Color Accent_Success = Color(60, 180, 75);
		Color Accent_Warning = Color(255, 128, 0);
		Color Accent_Danger = Color(255, 80, 80);

		Color Surface_Primary = Color(40, 40, 40);
		Color Surface_Secondary = Color(58, 58, 58);
		Color Surface_Tertiary = Color(25, 25, 25);



		Color Background_Panel = Surface_Primary;
		Color Background_Input = Surface_Secondary;
		Color Background_Popup = Color(45, 45, 45);

		Color Element_Normal = Surface_Secondary;
		Color Element_Hover = Color(80, 80, 80);
		Color Element_Pressed = Color(42, 42, 42);
		Color Element_Disabled = Color(80, 80, 80, 150);
		float Element_Padding = 4.0f;

		Color Border_Primary = Color(30, 30, 30);
		Color Border_Secondary = Color(40, 40, 40);
		Color Border_Hover = Color(100, 100, 100);
		Color Border_Selected = Accent_Primary;
		float Border_Thickness = 3.0f;
		float Border_HighlightThickness = 1.0f;

		Color Text_Main = Color(255, 255, 255);
		Color Text_Dim = Color(150, 150, 150);

		Color Text_Success = Accent_Success;
		Color Text_Warning = Accent_Warning;
		Color Text_Danger = Accent_Danger;

		Color NodeEditor_Background = Color(30, 30, 30);
		Color NodeEditor_GridLine = Color(50, 50, 50);
		Color NodeEditor_NodeBody = Color(45, 45, 45);
		Color NodeEditor_Selected = Color(255, 165, 0);

		FontAtlas* Font_Default = nullptr;

	};

	Theme& GetTheme();

}
