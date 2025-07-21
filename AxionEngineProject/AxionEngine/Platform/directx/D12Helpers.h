#pragma once

#include "axpch.h"

#include "AxionEngine/Platform/directx/D12FrameBuffer.h"

namespace Axion {

	class D12Helpers {
	public:

		static DXGI_FORMAT getD12ColorFormat(FrameBufferTextureFormat format);
		static DXGI_FORMAT getD12DepthFormat(FrameBufferDepthFormat format);

	};

}
