#pragma once

#include <string>
#include <unordered_map>

#include "MathTypes.h"
#include "Theme.h"

namespace Quartz {

	struct LanguageProfile {
		std::string name = "Plain Text";
		std::string singleLineComment = "//";
		std::string multiLineCommentStart = "/*";
		std::string multiLineCommentEnd = "*/";

		Silica::Color textColor = Silica::GetTheme().Text_Main;
		Silica::Color commentColor = Silica::Color(106, 153, 85, 255);
		Silica::Color stringColor = Silica::Color(206, 145, 120, 255);
		Silica::Color numberColor = Silica::Color(181, 206, 168, 255);
		Silica::Color functionColor = Silica::Color(220, 220, 170, 255);
		Silica::Color preprocessorColor = Silica::Color(155, 155, 155, 255);
		Silica::Color keyColor = Silica::Color(156, 220, 254, 255);

		std::unordered_map<std::string, Silica::Color> keywords;

		static LanguageProfile CPlusPlus();
		static LanguageProfile CSharp();
		static LanguageProfile HLSL();
		static LanguageProfile YAML();

	};

}
