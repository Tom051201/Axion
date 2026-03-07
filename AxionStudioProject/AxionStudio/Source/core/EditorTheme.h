#pragma once

namespace Axion {

	class EditorTheme {
	public:

		enum class Theme {
			RedBlack,
			Purple
		};

		static void setTheme(Theme theme);

		static Theme loadTheme(const char* filePath); // Maybe move those to a separate settings handler
		static void saveTheme(Theme theme, const char* filePath);

	};

}
