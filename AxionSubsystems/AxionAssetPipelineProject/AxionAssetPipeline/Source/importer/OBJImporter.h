#pragma once

#include <filesystem>
#include "AxionEngine/Source/render/Mesh.h"

namespace Axion::AAP {

	class OBJImporter {
	public:

		static MeshData extractMeshes(const std::filesystem::path& objPath);

	private:

		static void normalizeVector(DirectX::XMFLOAT3& v);

	};

}
