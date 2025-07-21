#pragma once

#include "axpch.h"

#include "AxionEngine/Source/render/SwapChainSpecification.h"

#include "AxionEngine/Platform/directx/D12FrameBuffer.h"

namespace Axion {

	class D12Helpers {
	public:

		static DXGI_FORMAT getD12TextureFormat(TextureFormat format);
		static DXGI_FORMAT getD12DepthStencilFormat(DepthStencilFormat format);

	};

}
