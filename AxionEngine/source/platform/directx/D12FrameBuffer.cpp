#include "axpch.h"
#include "D12FrameBuffer.h"

#include "Axion/render/GraphicsContext.h"

#include "platform/directx/D12Context.h"

namespace Axion {

	D12FrameBuffer::D12FrameBuffer(const FrameBufferSpecification& spec) : m_specification(spec) {
		m_context = static_cast<D12Context*>(GraphicsContext::get()->getNativeContext());

		// allocate rtv and srv once
		m_rtvHeapIndex = m_context->getRtvHeapWrapper().allocate();
		m_srvHeapIndex = m_context->getSrvHeapWrapper().allocate();

		resize(spec.width, spec.height);
	}

	D12FrameBuffer::~D12FrameBuffer() {
		release();
	}

	void D12FrameBuffer::release() {
		m_colorResource.Reset();
	}

	void D12FrameBuffer::resize(uint32_t width, uint32_t height) {
		release();
		auto* device = m_context->getDevice();
		auto* cmdList = m_context->getCommandList();

		m_specification.width = width;
		m_specification.height = height;

		D3D12_RESOURCE_DESC texDesc = {};
		texDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		texDesc.Width = m_specification.width;
		texDesc.Height = m_specification.height;
		texDesc.DepthOrArraySize = 1;
		texDesc.MipLevels = 1;
		texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		texDesc.SampleDesc.Count = 1;
		texDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
		texDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;

		D3D12_CLEAR_VALUE clearValue = {};
		clearValue.Format = texDesc.Format;
		clearValue.Color[0] = 0.0f;
		clearValue.Color[1] = 0.0f;
		clearValue.Color[2] = 0.0f;
		clearValue.Color[3] = 1.0f;

		CD3DX12_HEAP_PROPERTIES heapProps(D3D12_HEAP_TYPE_DEFAULT);
		HRESULT hr = device->CreateCommittedResource(
			&heapProps,
			D3D12_HEAP_FLAG_NONE,
			&texDesc,
			D3D12_RESOURCE_STATE_RENDER_TARGET,
			&clearValue,
			IID_PPV_ARGS(&m_colorResource)
		);
		AX_THROW_IF_FAILED_HR(hr, "Failed to create frame buffer resource");

		// RTV
		device->CreateRenderTargetView(m_colorResource.Get(), nullptr, m_context->getRtvHeapWrapper().getCpuHandle(m_rtvHeapIndex));

		// SRV
		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MipLevels = 1;
		m_context->getDevice()->CreateShaderResourceView(m_colorResource.Get(), &srvDesc, m_context->getSrvHeapWrapper().getCpuHandle(m_srvHeapIndex));

		#ifdef AX_DEBUG
		m_colorResource->SetName(L"FrameBuffer");
		#endif
	}

	void D12FrameBuffer::bind() const {
		auto* cmdList = m_context->getCommandList();
		
		if (m_currentState != D3D12_RESOURCE_STATE_RENDER_TARGET) {
			auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(
				m_colorResource.Get(),
				m_currentState,
				D3D12_RESOURCE_STATE_RENDER_TARGET
			);
			cmdList->ResourceBarrier(1, &barrier);
			m_currentState = D3D12_RESOURCE_STATE_RENDER_TARGET;
		}

		auto rtvHandle = m_context->getRtvHeapWrapper().getCpuHandle(m_rtvHeapIndex);
		cmdList->OMSetRenderTargets(1, &rtvHandle, FALSE, nullptr);
	}

	void D12FrameBuffer::unbind() const {
		auto* cmdList = m_context->getCommandList();

		if (m_currentState != D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE) {
			auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(
				m_colorResource.Get(),
				m_currentState,
				D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE
			);
			cmdList->ResourceBarrier(1, &barrier);
			m_currentState = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
		}
	}

	// using different values here than specified
	// in the resource description causes warning!
	void D12FrameBuffer::clear(const Vec4& clearColor) {
		auto* cmdList = m_context->getCommandList();
		auto rtvHandle = m_context->getRtvHeapWrapper().getCpuHandle(m_rtvHeapIndex);

		float color[] = { clearColor.x, clearColor.y, clearColor.z, clearColor.w };
		cmdList->ClearRenderTargetView(rtvHandle, color, 0, nullptr);
	}

}
