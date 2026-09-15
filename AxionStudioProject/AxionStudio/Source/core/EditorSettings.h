#pragma once

#include <filesystem>
#include <vector>
#include <string>
#include <functional>

namespace YAML {
	class Emitter;
	class Node;
}

namespace Axion {

	class EditorSettings {
	public:

		// -- Loading and saving --
		static bool load(const std::filesystem::path& filepath, std::function<void(const YAML::Node&)> deserializeSubPanels = nullptr);
		static bool save(const std::filesystem::path& filepath, std::function<void(YAML::Emitter&)> serializeSubPanels = nullptr);



		// -- Editor --
		static std::filesystem::path startupProjectPath;
		static std::vector<std::string> openTextEditors;
		static bool enableDiscordRPC;

		// -- Content Browser Panel --
		static bool contentBrowserShowContentArea;
		static bool contentBrowserShowVFSTree;
		static bool contentBrowserShowPhysicalTree;

		// -- Material Panel --
		static bool materialEditorInvertCamera;

		// -- Asset Library Panel --
		static std::vector<std::string> assetLibraryPaths;

	};

}
