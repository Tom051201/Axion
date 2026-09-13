#include "studiopch.h"
#include "EditorConfig.h"

namespace Axion {

	// -- Editor --
	std::filesystem::path EditorConfig::startupProjectPath = "None";
	bool EditorConfig::enableDiscordRPC = true;

	// -- Material Panel --
	bool EditorConfig::materialEditorInvertCamera = false;

}
