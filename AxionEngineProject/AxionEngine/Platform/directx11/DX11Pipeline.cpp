#include "axpch.h"
#include "DX11Pipeline.h"

#include "AxionEngine/Platform/directx11/DX11Context.h"
#include "AxionEngine/Platform/directx11/DX11Shader.h"
#include "AxionEngine/Platform/directx11/DX11DebugLayer.h"

namespace Axion {

	namespace Utils {
		static DXGI_FORMAT ShaderDataTypeToDXGIFormat(ShaderDataType type) {
			switch (type) {
			case ShaderDataType::Float:		return DXGI_FORMAT_R32_FLOAT;
			case ShaderDataType::Float2:	return DXGI_FORMAT_R32G32_FLOAT;
			case ShaderDataType::Float3:	return DXGI_FORMAT_R32G32B32_FLOAT;
			case ShaderDataType::Float4:	return DXGI_FORMAT_R32G32B32A32_FLOAT;
			case ShaderDataType::Int:		return DXGI_FORMAT_R32_SINT;
			case ShaderDataType::Int2:		return DXGI_FORMAT_R32G32_SINT;
			case ShaderDataType::Int3:		return DXGI_FORMAT_R32G32B32_SINT;
			case ShaderDataType::Int4:		return DXGI_FORMAT_R32G32B32A32_SINT;
			}
			return DXGI_FORMAT_UNKNOWN;
		}

		static D3D11_CULL_MODE CullModeToDX11(CullMode mode) {
			switch (mode) {
			case CullMode::None: return D3D11_CULL_NONE;
			case CullMode::Front: return D3D11_CULL_FRONT;
			case CullMode::Back: return D3D11_CULL_BACK;
			}
			return D3D11_CULL_BACK;
		}

		static D3D11_COMPARISON_FUNC DepthCompareToDX11(DepthCompare func) {
			switch (func) {
			case DepthCompare::Never: return D3D11_COMPARISON_NEVER;
			case DepthCompare::Less: return D3D11_COMPARISON_LESS;
			case DepthCompare::Equal: return D3D11_COMPARISON_EQUAL;
			case DepthCompare::LessEqual: return D3D11_COMPARISON_LESS_EQUAL;
			case DepthCompare::Greater: return D3D11_COMPARISON_GREATER;
			case DepthCompare::NotEqual: return D3D11_COMPARISON_NOT_EQUAL;
			case DepthCompare::GreaterEqual: return D3D11_COMPARISON_GREATER_EQUAL;
			case DepthCompare::Always: return D3D11_COMPARISON_ALWAYS;
			}
			return D3D11_COMPARISON_LESS;
		}

		static D3D11_PRIMITIVE_TOPOLOGY TopologyToDX11(PrimitiveTopology topology) {
			switch (topology) {
			case PrimitiveTopology::PointList: return D3D11_PRIMITIVE_TOPOLOGY_POINTLIST;
			case PrimitiveTopology::LineList: return D3D11_PRIMITIVE_TOPOLOGY_LINELIST;
			case PrimitiveTopology::LineStrip: return D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP;
			case PrimitiveTopology::TriangleList: return D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
			case PrimitiveTopology::TriangleStrip: return D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;
			}
			return D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
		}
	}

	DX11Pipeline::DX11Pipeline(const PipelineSpecification& spec)
		: m_specification(spec) {
		
		auto* device = static_cast<DX11Context*>(GraphicsContext::get()->getNativeContext())->getDevice();

		AX_CORE_ASSERT(m_specification.shader, "PipelineSpecification must have a valid shader!");
		Ref<DX11Shader> shader = m_specification.shader.staticAs<DX11Shader>();

		HRESULT hr;

		// ----- Input Layout -----
		std::vector<D3D11_INPUT_ELEMENT_DESC> inputElements;
		const auto& layout = m_specification.vertexLayout.getElements();
		std::unordered_map<std::string, uint32_t> semanticCounts;

		if (layout.size() > 0) {
			for (const auto& element : layout) {
				D3D11_INPUT_ELEMENT_DESC desc = {};
				desc.SemanticName = element.name.c_str();
				desc.SemanticIndex = semanticCounts[element.name]++;
				desc.Format = Utils::ShaderDataTypeToDXGIFormat(element.type);
				desc.InputSlot = element.instanced ? 1 : 0;
				desc.AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
				desc.InputSlotClass = element.instanced ? D3D11_INPUT_PER_INSTANCE_DATA : D3D11_INPUT_PER_VERTEX_DATA;
				desc.InstanceDataStepRate = element.instanced ? 1 : 0;

				inputElements.push_back(desc);
			}

			hr = device->CreateInputLayout(
				inputElements.data(), (UINT)inputElements.size(),
				shader->getVertexBlob()->GetBufferPointer(),
				shader->getVertexBlob()->GetBufferSize(),
				&m_inputLayout
			);
			AX_THROW_IF_FAILED_HR(hr, "Failed to create DX11 Input Layout");
		}

		// ----- Rasterizer State -----
		D3D11_RASTERIZER_DESC rasterDesc = {};
		rasterDesc.FillMode = D3D11_FILL_SOLID;
		rasterDesc.CullMode = Utils::CullModeToDX11(m_specification.cullMode);
		rasterDesc.FrontCounterClockwise = FALSE;
		rasterDesc.DepthClipEnable = TRUE;

		hr = device->CreateRasterizerState(&rasterDesc, &m_rasterizerState);
		AX_THROW_IF_FAILED_HR(hr, "Failed to create DX11 Rasterizer State");

		// ----- Blend State -----
		D3D11_BLEND_DESC blendDesc = {};
		blendDesc.AlphaToCoverageEnable = FALSE;
		blendDesc.IndependentBlendEnable = TRUE;

		bool isIntegerFormat = (m_specification.colorFormat == ColorFormat::RED_INTEGER);

		// -- Target 0 Color --
		blendDesc.RenderTarget[0].BlendEnable = (m_specification.numRenderTargets > 0 && !isIntegerFormat) ? TRUE : FALSE;
		blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
		blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
		blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
		blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
		blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
		blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
		blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

		// -- Target 1 Entity ID --
		if (m_specification.numRenderTargets > 1) {
			blendDesc.RenderTarget[1].BlendEnable = FALSE;
			blendDesc.RenderTarget[1].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
		}

		hr = device->CreateBlendState(&blendDesc, &m_blendState);
		AX_THROW_IF_FAILED_HR(hr, "Failed to create DX11 Blend State");

		// ----- Depth Stencil State -----
		D3D11_DEPTH_STENCIL_DESC depthDesc = {};
		depthDesc.DepthEnable = m_specification.depthTest ? TRUE : FALSE;
		depthDesc.DepthWriteMask = m_specification.depthWrite ? D3D11_DEPTH_WRITE_MASK_ALL : D3D11_DEPTH_WRITE_MASK_ZERO;
		depthDesc.DepthFunc = Utils::DepthCompareToDX11(m_specification.depthFunction);
		depthDesc.StencilEnable = m_specification.stencilEnabled ? TRUE : FALSE;

		hr = device->CreateDepthStencilState(&depthDesc, &m_depthStencilState);
		AX_THROW_IF_FAILED_HR(hr, "Failed to create DX11 Depth Stencil State");

		// ----- Topology -----
		m_topology = Utils::TopologyToDX11(m_specification.topology);

		// ----- Debug Naming -----
		#ifdef AX_DEBUG
		if (m_inputLayout) DX11DebugLayer::setName(m_inputLayout.Get(), "DX11 InputLayout");
		DX11DebugLayer::setName(m_rasterizerState.Get(), "DX11 RasterizerState");
		DX11DebugLayer::setName(m_blendState.Get(), "DX11 BlendState");
		DX11DebugLayer::setName(m_depthStencilState.Get(), "DX11 DepthStencilState");
		#endif
	}

	DX11Pipeline::~DX11Pipeline() {
		release();
	}

	void DX11Pipeline::release() {
		m_inputLayout.Reset();
		m_rasterizerState.Reset();
		m_blendState.Reset();
		m_depthStencilState.Reset();
	}

	void DX11Pipeline::bind() {
		auto* ctx = static_cast<DX11Context*>(GraphicsContext::get()->getNativeContext())->getDeviceContext();

		// -- Bind State Objects --
		if (m_inputLayout) ctx->IASetInputLayout(m_inputLayout.Get());
		ctx->RSSetState(m_rasterizerState.Get());
		ctx->OMSetBlendState(m_blendState.Get(), nullptr, 0xffffffff);
		ctx->OMSetDepthStencilState(m_depthStencilState.Get(), 0);
		ctx->IASetPrimitiveTopology(m_topology);

		// -- Bind Shader --
		if (m_specification.shader) {
			m_specification.shader->bind();
		}
	}

	void DX11Pipeline::unbind() {
		auto* ctx = static_cast<DX11Context*>(GraphicsContext::get()->getNativeContext())->getDeviceContext();

		// -- Unbind State Objects --
		ctx->IASetInputLayout(nullptr);
		ctx->RSSetState(nullptr);
		ctx->OMSetBlendState(nullptr, nullptr, 0xffffffff);
		ctx->OMSetDepthStencilState(nullptr, 0);

		// -- Unbind Shader --
		if (m_specification.shader) {
			m_specification.shader->unbind();
		}
	}

}
