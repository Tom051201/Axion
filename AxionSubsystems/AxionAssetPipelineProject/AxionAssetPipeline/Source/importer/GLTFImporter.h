#pragma once

#include <filesystem>

#include "AxionEngine/Source/render/Mesh.h"
#include "AxionEngine/Source/render/SkeletalMesh.h"
#include "AxionEngine/Source/scene/Animation.h"

namespace Axion::AAP {

	class GLTFImporter {
	public:

		static void import(const std::filesystem::path& path);

		static MeshData extractMeshes(const std::filesystem::path& path);
		static SkeletalMeshData extractSkeletalMesh(const std::filesystem::path& path);
		static Ref<AnimationClip> extractAnimation(const std::filesystem::path& path);

	};

}
