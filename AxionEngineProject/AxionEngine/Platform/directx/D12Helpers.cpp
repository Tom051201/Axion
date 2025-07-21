#include "axpch.h"
#include "D12Helpers.h"

namespace Axion {

	DXGI_FORMAT D12Helpers::getD12TextureFormat(TextureFormat format) {
		switch (format) {
		case TextureFormat::None:			return DXGI_FORMAT_UNKNOWN;
		case TextureFormat::RGBA8:			return DXGI_FORMAT_R8G8B8A8_UNORM;
		case TextureFormat::RED_INTEGER:	return DXGI_FORMAT_R32_SINT;
		case TextureFormat::RGBA16F:		return DXGI_FORMAT_R16G16B16A16_FLOAT;
		case TextureFormat::BGRA8:			return DXGI_FORMAT_B8G8R8A8_UNORM;
		case TextureFormat::RGB10A2:		return DXGI_FORMAT_R10G10B10A2_UNORM;
		default:							return DXGI_FORMAT_UNKNOWN;
		}
	}

	DXGI_FORMAT D12Helpers::getD12DepthStencilFormat(DepthStencilFormat format) {
		switch (format) {
		case DepthStencilFormat::None:				return DXGI_FORMAT_UNKNOWN;
		case DepthStencilFormat::DEPTH24_STENCIL8:	return DXGI_FORMAT_D24_UNORM_S8_UINT;
		case DepthStencilFormat::DEPTH32F:			return DXGI_FORMAT_D32_FLOAT;
		case DepthStencilFormat::DEPTH32F_STENCIL8:	return DXGI_FORMAT_D32_FLOAT_S8X24_UINT;
		case DepthStencilFormat::DEPTH16:			return DXGI_FORMAT_D16_UNORM;
		default:									return DXGI_FORMAT_UNKNOWN;
		}
	}

}
