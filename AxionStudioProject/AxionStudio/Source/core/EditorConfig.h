#pragma once

#include <filesystem>

namespace Axion {

	class EditorConfig {
	public:

		// -- Editor --
		static std::filesystem::path startupProjectPath;
		static bool enableDiscordRPC;

		// -- Material Panel --
		static bool materialEditorInvertCamera;

	};

}
