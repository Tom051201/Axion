#include "axpch.h"
#include "D12Pipeline.h"

#include "AxionEngine/Platform/directx/D12Context.h"
#include "AxionEngine/Platform/directx/D12Helpers.h"
#include "AxionEngine/Platform/directx/D12Shader.h"

namespace Axion {

	D12Pipeline::D12Pipeline(const PipelineSpecification& spec)
		: m_specification(spec) {

		auto* device = static_cast<D12Context*>(GraphicsContext::get()->getNativeContext())->getDevice();

		AX_CORE_ASSERT(m_specification.shader, "PipelineSpecification must have a valid shader!");
		Ref<D12Shader> d12Shader = std::static_pointer_cast<D12Shader>(m_specification.shader);

		std::vector<D3D12_INPUT_ELEMENT_DESC> inputElements;
		const auto& layout = m_specification.vertexLayout.getElements();
		std::unordered_map<std::string, uint32_t> semanticCounts;

		AX_CORE_ASSERT(layout.size() > 0, "No buffer layout specified");

		for (const auto& element : layout) {
			D3D12_INPUT_ELEMENT_DESC desc = {};
			desc.SemanticName = element.name.c_str();
			desc.SemanticIndex = semanticCounts[element.name]++;
			desc.Format = D12Helpers::ShaderDataTypeToDXGIFormat(element.type);
			desc.InputSlot = element.instanced ? 1 : 0;
			desc.InputSlotClass = element.instanced ? D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA : D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;
			desc.InstanceDataStepRate = element.instanced ? 1 : 0;
			desc.AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;

			inputElements.push_back(desc);
		}

		D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
		psoDesc.InputLayout = { inputElements.data(), (UINT)inputElements.size() };
		psoDesc.pRootSignature = d12Shader->getRootSignature();
		psoDesc.VS = CD3DX12_SHADER_BYTECODE(d12Shader->getVertexBlob().Get());
		psoDesc.PS = CD3DX12_SHADER_BYTECODE(d12Shader->getPixelBlob().Get());

		psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
		psoDesc.RasterizerState.CullMode = D12Helpers::toD12CullMode(m_specification.cullMode);

		D3D12_RENDER_TARGET_BLEND_DESC blendDesc = {};
		blendDesc.BlendEnable = TRUE;
		blendDesc.LogicOpEnable = FALSE;
		blendDesc.SrcBlend = D3D12_BLEND_SRC_ALPHA;
		blendDesc.DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
		blendDesc.BlendOp = D3D12_BLEND_OP_ADD;
		blendDesc.SrcBlendAlpha = D3D12_BLEND_ONE;
		blendDesc.DestBlendAlpha = D3D12_BLEND_ZERO;
		blendDesc.BlendOpAlpha = D3D12_BLEND_OP_ADD;
		blendDesc.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

		psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
		psoDesc.BlendState.RenderTarget[0] = blendDesc;

		psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
		psoDesc.DepthStencilState.DepthEnable = m_specification.depthTest ? TRUE : FALSE;
		psoDesc.DepthStencilState.DepthWriteMask = m_specification.depthWrite ? D3D12_DEPTH_WRITE_MASK_ALL : D3D12_DEPTH_WRITE_MASK_ZERO;
		psoDesc.DepthStencilState.DepthFunc = D12Helpers::toD12DepthComparisonFunction(m_specification.depthFunction);
		psoDesc.DSVFormat = D12Helpers::toD12DepthStencilFormat(m_specification.depthStencilFormat);
		psoDesc.DepthStencilState.StencilEnable = m_specification.stencilEnabled ? TRUE : FALSE;

		psoDesc.SampleMask = UINT_MAX;
		psoDesc.PrimitiveTopologyType = D12Helpers::toD12ToplogyType(m_specification.topology);
		psoDesc.NumRenderTargets = m_specification.numRenderTargets;
		if (m_specification.numRenderTargets > 0) {
			psoDesc.RTVFormats[0] = D12Helpers::toD12ColorFormat(m_specification.colorFormat);
		}
		psoDesc.SampleDesc.Count = m_specification.sampleCount;

		HRESULT hr = device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_pipelineState));
		AX_THROW_IF_FAILED_HR(hr, "Failed to create graphics pipeline state");

		#ifdef AX_DEBUG
		m_pipelineState->SetName(L"PipelineState");
		#endif
	}

	D12Pipeline::~D12Pipeline() {
		release();
	}

	void D12Pipeline::release() {
		m_pipelineState.Reset();
	}

	void D12Pipeline::bind() {
		auto* cmdList = static_cast<D12Context*>(GraphicsContext::get()->getNativeContext())->getCommandList();
		Ref<D12Shader> d12Shader = std::static_pointer_cast<D12Shader>(m_specification.shader);

		cmdList->SetGraphicsRootSignature(d12Shader->getRootSignature());
		cmdList->SetPipelineState(m_pipelineState.Get());
		cmdList->IASetPrimitiveTopology(D12Helpers::toD12Topology(m_specification.topology));
	}

	void D12Pipeline::unbind() {}

}
