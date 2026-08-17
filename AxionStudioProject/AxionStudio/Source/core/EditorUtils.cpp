#include "studiopch.h"
#include "EditorUtils.h"

namespace Axion {

	bool EditorUtils::isEngineAssetExtension(const std::filesystem::path& path) {
		std::string ext = path.extension().string();
		std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
		return isEngineAssetExtension(ext);
	}

	bool EditorUtils::isEngineAssetExtension(const std::string& extension) {
		std::string ext = extension;
		std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
		return ext == ".axmesh" || ext == ".axsky" || ext == ".axshader" || ext == ".axmat" || ext == ".axaudio" || ext == ".axtex" || ext == ".axpmat" || ext == ".axprefab" || ext == ".axpso" || ext == ".axscene" || ext == ".axtcube" || ext == ".axanim" || ext == ".axskelmesh" || ext == ".axvs";
	}

	bool EditorUtils::isTextEditorFile(const std::string& extension) {
		std::string ext = extension;
		std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
		return ext == ".cs" || ext == ".h" || ext == ".cpp" || ext == ".hlsl" || ext == ".glsl" || ext == ".txt" || ext == ".yaml" || ext == ".ini" || ext == ".axvslayout" || isEngineAssetExtension(ext);
	}

}
