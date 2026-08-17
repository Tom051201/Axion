#pragma once

#include <d3d12.h>
#include <dxgiformat.h>

#include "AxionEngine/Source/graphics/SwapChainSpecification.h"
#include "AxionEngine/Source/graphics/Pipeline.h"
#include "AxionEngine/Source/graphics/Buffers.h"
#include "AxionEngine/Source/graphics/Formats.h"

namespace Axion {

	class DX12Helpers {
	public:

		static DXGI_FORMAT toDX12ColorFormat(ColorFormat format);
		static DXGI_FORMAT toDX12DepthStencilFormat(DepthStencilFormat format);

		static D3D12_CULL_MODE toDX12CullMode(CullMode mode);
		static D3D12_COMPARISON_FUNC toDX12DepthComparisonFunction(DepthCompare function);
		static D3D12_PRIMITIVE_TOPOLOGY_TYPE toDX12ToplogyType(PrimitiveTopology topology);
		static D3D12_PRIMITIVE_TOPOLOGY toDX12Topology(PrimitiveTopology topology);

		static DXGI_FORMAT ShaderDataTypeToDXGIFormat(ShaderDataType type);

	};

}
