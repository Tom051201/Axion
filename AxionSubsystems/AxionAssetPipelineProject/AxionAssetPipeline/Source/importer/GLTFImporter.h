#pragma once

#include <filesystem>

#include "AxionEngine/Source/render/Mesh.h"

namespace Axion::AAP {

	class GLTFImporter {
	public:

		static void import(const std::filesystem::path& path);

		static MeshData extractMeshes(const std::filesystem::path& path);

	};

}
