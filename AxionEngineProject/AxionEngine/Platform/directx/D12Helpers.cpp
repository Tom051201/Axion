#include "axpch.h"
#include "D12Helpers.h"

namespace Axion {

	DXGI_FORMAT D12Helpers::toD12ColorFormat(ColorFormat format) {
		switch (format) {
		case ColorFormat::None:			return DXGI_FORMAT_UNKNOWN;
		case ColorFormat::RGBA8:		return DXGI_FORMAT_R8G8B8A8_UNORM;
		case ColorFormat::RED_INTEGER:	return DXGI_FORMAT_R32_SINT;
		case ColorFormat::RGBA16F:		return DXGI_FORMAT_R16G16B16A16_FLOAT;
		case ColorFormat::BGRA8:		return DXGI_FORMAT_B8G8R8A8_UNORM;
		case ColorFormat::RGB10A2:		return DXGI_FORMAT_R10G10B10A2_UNORM;
		default:						return DXGI_FORMAT_UNKNOWN;
		}
	}

	DXGI_FORMAT D12Helpers::toD12DepthStencilFormat(DepthStencilFormat format) {
		switch (format) {
		case DepthStencilFormat::None:				return DXGI_FORMAT_UNKNOWN;
		case DepthStencilFormat::DEPTH24_STENCIL8:	return DXGI_FORMAT_D24_UNORM_S8_UINT;
		case DepthStencilFormat::DEPTH32F:			return DXGI_FORMAT_D32_FLOAT;
		case DepthStencilFormat::DEPTH32F_STENCIL8:	return DXGI_FORMAT_D32_FLOAT_S8X24_UINT;
		case DepthStencilFormat::DEPTH16:			return DXGI_FORMAT_D16_UNORM;
		default:									return DXGI_FORMAT_UNKNOWN;
		}
	}

	D3D12_CULL_MODE D12Helpers::toD12CullMode(CullMode mode) {
		switch (mode) {
		case CullMode::None:		return D3D12_CULL_MODE_NONE;
		case CullMode::Front:	return D3D12_CULL_MODE_FRONT;
		case CullMode::Back:		return D3D12_CULL_MODE_BACK;
		default:						return D3D12_CULL_MODE_BACK;
		}
	}

	D3D12_COMPARISON_FUNC D12Helpers::toD12DepthComparisonFunction(DepthCompare function) {
		switch (function) {
		case DepthCompare::Never:		return D3D12_COMPARISON_FUNC_NEVER;
		case DepthCompare::Less:			return D3D12_COMPARISON_FUNC_LESS;
		case DepthCompare::Equal:		return D3D12_COMPARISON_FUNC_EQUAL;
		case DepthCompare::LessEqual:	return D3D12_COMPARISON_FUNC_LESS_EQUAL;
		case DepthCompare::Greater:		return D3D12_COMPARISON_FUNC_GREATER;
		case DepthCompare::NotEqual:		return D3D12_COMPARISON_FUNC_NOT_EQUAL;
		case DepthCompare::GreaterEqual:	return D3D12_COMPARISON_FUNC_GREATER_EQUAL;
		case DepthCompare::Always:		return D3D12_COMPARISON_FUNC_ALWAYS;
		default:								return D3D12_COMPARISON_FUNC_LESS;
		}
	}

	D3D12_PRIMITIVE_TOPOLOGY_TYPE D12Helpers::toD12ToplogyType(PrimitiveTopology topology) {
		switch (topology) {
		case PrimitiveTopology::PointList:		return D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT;
		case PrimitiveTopology::LineList:
		case PrimitiveTopology::LineStrip:		return D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE;
		case PrimitiveTopology::TriangleList:
		case PrimitiveTopology::TriangleStrip:	return D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		default:								return D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		}
	}

	D3D12_PRIMITIVE_TOPOLOGY D12Helpers::toD12Topology(PrimitiveTopology topology) {
		switch (topology) {
		case PrimitiveTopology::PointList:		return D3D_PRIMITIVE_TOPOLOGY_POINTLIST;
		case PrimitiveTopology::LineList:		return D3D_PRIMITIVE_TOPOLOGY_LINELIST;
		case PrimitiveTopology::LineStrip:		return D3D_PRIMITIVE_TOPOLOGY_LINESTRIP;
		case PrimitiveTopology::TriangleList:	return D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
		case PrimitiveTopology::TriangleStrip:	return D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;
		default:								return D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
		}
	}

	DXGI_FORMAT D12Helpers::ShaderDataTypeToDXGIFormat(ShaderDataType type) {
		switch (type) {
		case ShaderDataType::None:		AX_CORE_LOG_WARN("None as Shader data type is not recommended"); return DXGI_FORMAT_UNKNOWN;
		case ShaderDataType::Float:		return DXGI_FORMAT_R32_FLOAT;
		case ShaderDataType::Float2:	return DXGI_FORMAT_R32G32_FLOAT;
		case ShaderDataType::Float3:	return DXGI_FORMAT_R32G32B32_FLOAT;
		case ShaderDataType::Float4:	return DXGI_FORMAT_R32G32B32A32_FLOAT;
		case ShaderDataType::Int:		return DXGI_FORMAT_R32_SINT;
		case ShaderDataType::Int2:		return DXGI_FORMAT_R32G32_SINT;
		case ShaderDataType::Int3:		return DXGI_FORMAT_R32G32B32_SINT;
		case ShaderDataType::Int4:		return DXGI_FORMAT_R32G32B32A32_SINT;
		case ShaderDataType::Bool:		return DXGI_FORMAT_R8_UINT;
		default:						AX_CORE_LOG_WARN("Unknown ShaderDataType"); return DXGI_FORMAT_UNKNOWN;
		}
	}

}
