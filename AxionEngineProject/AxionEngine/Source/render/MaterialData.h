#pragma once

#include "AxionEngine/Source/core/Math.h"

namespace Axion {

	struct MaterialProperties {
		Vec4 albedoColor = Vec4::one();

		float metalness = 0.0f;
		float roughness = 0.0f;
		float emissionStrength = 0.0f;
		float tiling = 1.0f;

		float useNormalMap = 0.0f;
		float useMetalnessMap = 0.0f;
		float useRoughnessMap = 0.0f;
		float useOcclusionMap = 0.0f;

	};

	enum class TextureSlot {
		Albedo = 0,
		Normal,
		Metalness,
		Roughness,
		Occlusion,
		Emissive,
		COUNT
	};

}
