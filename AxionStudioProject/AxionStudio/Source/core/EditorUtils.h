#pragma once

#include <filesystem>
#include <string>

namespace Axion {

	class EditorUtils {
	public:

		static bool isEngineAssetExtension(const std::filesystem::path& path);
		static bool isEngineAssetExtension(const std::string& extension);

		static bool isTextEditorFile(const std::string& extension);

	};

}
