#pragma once

#include "AxionEngine/Source/core/UUID.h"

namespace Axion {

	struct BinaryAssetHeader {
		char magic[4] = { 'A', 'X', 'B', 'N' };
		uint32_t version = 1;
		AssetType type = AssetType::None;
		UUID uuid;
	};

	struct MaterialBinaryHeader {
		BinaryAssetHeader assetHeader;
		MaterialProperties properties;
		UUID pipelineUUID;
		uint32_t textureCount = 0;
	};

	struct PipelineBinaryHeader {
		BinaryAssetHeader assetHeader;
		UUID shaderUUID;

		uint32_t colorFormat;
		uint32_t depthStencilFormat;
		uint8_t depthTest;
		uint8_t depthWrite;
		uint32_t depthFunction;
		uint8_t stencilEnabled;
		uint32_t sampleCount;
		uint32_t cullMode;
		uint32_t topology;
		uint32_t numRenderTargets;

		uint32_t bufferElementCount;
	};

	struct ShaderBinaryHeader {
		BinaryAssetHeader assetHeader;
		uint32_t batchTextures;
		uint64_t vsSize;
		uint64_t psSize;
	};

	struct SkyboxBinaryHeader {
		BinaryAssetHeader assetHeader;
		UUID textureCubeUUID;
		UUID pipelineUUID;
	};

}
