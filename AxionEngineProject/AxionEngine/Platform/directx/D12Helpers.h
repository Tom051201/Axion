#pragma once

#include "axpch.h"

#include "AxionEngine/Source/render/SwapChainSpecification.h"
#include "AxionEngine/Source/render/Shader.h"


namespace Axion {

	class D12Helpers {
	public:

		static DXGI_FORMAT toD12ColorFormat(ColorFormat format);
		static DXGI_FORMAT toD12DepthStencilFormat(DepthStencilFormat format);

		static D3D12_CULL_MODE toD12CullMode(CullMode mode);
		static D3D12_COMPARISON_FUNC toD12DepthComparisonFunction(DepthCompare function);
		static D3D12_PRIMITIVE_TOPOLOGY_TYPE toD12ToplogyType(PrimitiveTopology topology);
		static D3D12_PRIMITIVE_TOPOLOGY toD12Topology(PrimitiveTopology topology);

	};

}
