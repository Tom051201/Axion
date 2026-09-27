#pragma once

#include <filesystem>

#include "AxionEngine/Source/core/Ref.h"
#include "AxionEngine/Source/graphics/Mesh.h"

namespace Axion::AAP {

	class PhysicsCooker {
	public:

		static bool cookConvex(const Ref<Mesh>& visualMesh, uint32_t vertexLimit, const std::filesystem::path& outputPath);
		static bool cookTriangle(const Ref<Mesh>& visualMesh, const std::filesystem::path& outputPath);

	};

}
