#pragma once

#include <cstdint>

#include "AxionEngine/Source/AxionSettings.h"
#include "AxionEngine/Source/render/Formats.h"

namespace Axion {

	struct SwapChainSpecification {
		uint32_t width = 1280;
		uint32_t height = 720;
		uint32_t bufferCount = AX_MAX_SWAPCHAIN_BUFFERS;
		TextureFormat backBufferFormat = TextureFormat::RGBA8;
		DepthStencilFormat depthBufferFormat = DepthStencilFormat::DEPTH32F;
	};

}
