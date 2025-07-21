#include "axpch.h"
#include "D12Helpers.h"

namespace Axion {

	DXGI_FORMAT D12Helpers::getD12ColorFormat(FrameBufferTextureFormat format) {
		switch (format) {
		case FrameBufferTextureFormat::None:			return DXGI_FORMAT_UNKNOWN;
		case FrameBufferTextureFormat::RGBA8:			return DXGI_FORMAT_R8G8B8A8_UNORM;
		case FrameBufferTextureFormat::RED_INTEGER:		return DXGI_FORMAT_R32_SINT;
		case FrameBufferTextureFormat::RGBA16F:			return DXGI_FORMAT_R16G16B16A16_FLOAT;
		case FrameBufferTextureFormat::BGRA8:			return DXGI_FORMAT_B8G8R8A8_UNORM;
		case FrameBufferTextureFormat::RGB10A2:			return DXGI_FORMAT_R10G10B10A2_UNORM;
		default:										return DXGI_FORMAT_UNKNOWN;
		}
	}

	DXGI_FORMAT D12Helpers::getD12DepthFormat(FrameBufferDepthFormat format) {
		switch (format) {
		case FrameBufferDepthFormat::None:				return DXGI_FORMAT_UNKNOWN;
		case FrameBufferDepthFormat::DEPTH24_STENCIL8:	return DXGI_FORMAT_D24_UNORM_S8_UINT;
		case FrameBufferDepthFormat::DEPTH32F:			return DXGI_FORMAT_D32_FLOAT;
		case FrameBufferDepthFormat::DEPTH32F_STENCIL8:	return DXGI_FORMAT_D32_FLOAT_S8X24_UINT;
		case FrameBufferDepthFormat::DEPTH16:			return DXGI_FORMAT_D16_UNORM;
		default:										return DXGI_FORMAT_UNKNOWN;
		}
	}

}
