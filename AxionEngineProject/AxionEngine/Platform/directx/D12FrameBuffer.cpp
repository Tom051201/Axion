#include "axpch.h"
#include "D12FrameBuffer.h"

#include "AxionEngine/Platform/directx/D12Context.h"
#include "AxionEngine/Platform/directx/D12Helpers.h"

namespace Axion {

	D12FrameBuffer::D12FrameBuffer(const FrameBufferSpecification& spec) : m_specification(spec) {
		m_context = static_cast<D12Context*>(GraphicsContext::get()->getNativeContext());
		AX_CORE_ASSERT(m_context, "Failed to acquire D12 context");
		m_currentState = D3D12_RESOURCE_STATE_RENDER_TARGET;

		try {
			resize(spec.width, spec.height);
			m_allocated = true;
		}
		catch (...) {
			AX_CORE_LOG_ERROR("Error creating buffers");
			throw;
		}
	}

	D12FrameBuffer::~D12FrameBuffer() {
		release();
	}

	void D12FrameBuffer::release() {
		if (!m_allocated) return;

		if (m_colorResource) {
			m_colorResource.Reset();
			m_context->getRtvHeapWrapper().free(m_rtvHeapIndex);
			m_context->getSrvHeapWrapper().free(m_srvHeapIndex);
		}

		if (m_depthResource) {
			m_depthResource.Reset();
			m_context->getDsvHeapWrapper().free(m_dsvHeapIndex);
		}
	}

	void D12FrameBuffer::resize(uint32_t width, uint32_t height) {

		// secures that the width and height are at
		// least 1px otherwise this failes
		width = std::max(1u, width);
		height = std::max(1u, height);

		release();

		// reallocate descriptor heap indices
		m_rtvHeapIndex = m_context->getRtvHeapWrapper().allocate();
		m_srvHeapIndex = m_context->getSrvHeapWrapper().allocate();
		m_dsvHeapIndex = m_context->getDsvHeapWrapper().allocate();

		auto* device = m_context->getDevice();

		m_specification.width = width;
		m_specification.height = height;

		// texture
		D3D12_RESOURCE_DESC texDesc = {};
		texDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		texDesc.Width = m_specification.width;
		texDesc.Height = m_specification.height;
		texDesc.DepthOrArraySize = 1;
		texDesc.MipLevels = 1;
		texDesc.Format = D12Helpers::toD12ColorFormat(m_specification.textureFormat);
		texDesc.SampleDesc.Count = 1;
		texDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
		texDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;

		D3D12_CLEAR_VALUE clearValue = {};
		clearValue.Format = texDesc.Format;
		clearValue.Color[0] = m_specification.clearColor.x;
		clearValue.Color[1] = m_specification.clearColor.y;
		clearValue.Color[2] = m_specification.clearColor.z;
		clearValue.Color[3] = m_specification.clearColor.w;

		CD3DX12_HEAP_PROPERTIES heapProps(D3D12_HEAP_TYPE_DEFAULT);
		HRESULT hr = device->CreateCommittedResource(
			&heapProps,
			D3D12_HEAP_FLAG_NONE,
			&texDesc,
			m_currentState,
			&clearValue,
			IID_PPV_ARGS(&m_colorResource)
		);
		AX_THROW_IF_FAILED_HR(hr, "Failed to create frame buffer texture resource");

		// depth
		DXGI_FORMAT depthFormat = D12Helpers::toD12DepthStencilFormat(m_specification.depthStencilFormat);
		if (depthFormat == DXGI_FORMAT_UNKNOWN) {
			AX_CORE_LOG_WARN("Attempting to create framebuffer with unknown depth format");
		}

		CD3DX12_RESOURCE_DESC depthDesc = CD3DX12_RESOURCE_DESC::Tex2D(
			depthFormat,
			m_specification.width,
			m_specification.height,
			1, 1
		);
		depthDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

		D3D12_CLEAR_VALUE depthClearValue = {};
		depthClearValue.Format = depthFormat;
		depthClearValue.DepthStencil.Depth = 1.0f;
		depthClearValue.DepthStencil.Stencil = 0;

		CD3DX12_HEAP_PROPERTIES depthHeapProps(D3D12_HEAP_TYPE_DEFAULT);
		hr = device->CreateCommittedResource(
			&depthHeapProps,
			D3D12_HEAP_FLAG_NONE,
			&depthDesc,
			D3D12_RESOURCE_STATE_DEPTH_WRITE,
			&depthClearValue,
			IID_PPV_ARGS(&m_depthResource)
		);
		AX_THROW_IF_FAILED_HR(hr, "Failed to create frame buffer depth resource");

		D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
		dsvDesc.Format = depthFormat;
		dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
		dsvDesc.Flags = D3D12_DSV_FLAG_NONE;

		device->CreateDepthStencilView(
			m_depthResource.Get(),
			&dsvDesc,
			m_context->getDsvHeapWrapper().getCpuHandle(m_dsvHeapIndex)
		);

		// RTV
		device->CreateRenderTargetView(m_colorResource.Get(), nullptr, m_context->getRtvHeapWrapper().getCpuHandle(m_rtvHeapIndex));

		// SRV
		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srvDesc.Format = texDesc.Format;
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MipLevels = 1;
		m_context->getDevice()->CreateShaderResourceView(m_colorResource.Get(), &srvDesc, m_context->getSrvHeapWrapper().getCpuHandle(m_srvHeapIndex));

		#ifdef AX_DEBUG
		m_colorResource->SetName(L"FrameBufferColor");
		m_depthResource->SetName(L"FrameBufferDepth");
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
		auto dsvHandle = m_context->getDsvHeapWrapper().getCpuHandle(m_dsvHeapIndex);
		cmdList->OMSetRenderTargets(1, &rtvHandle, FALSE, &dsvHandle);
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
	// Also does setting colors here not change the
	// specification!
	void D12FrameBuffer::clear(const Vec4& clearColor) {
		auto* cmdList = m_context->getCommandList();
		auto rtvHandle = m_context->getRtvHeapWrapper().getCpuHandle(m_rtvHeapIndex);
		auto dsvHandle = m_context->getDsvHeapWrapper().getCpuHandle(m_dsvHeapIndex);

		#ifdef AX_DEBUG
		if (clearColor != m_specification.clearColor) {
			AX_CORE_LOG_WARN("Clearing the D12Framebuffer with not optimized color!");
		}
		#endif

		float color[] = { clearColor.x, clearColor.y, clearColor.z, clearColor.w };
		cmdList->ClearRenderTargetView(rtvHandle, color, 0, nullptr);
		cmdList->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
	}

	void D12FrameBuffer::clear() {
		clear(m_specification.clearColor);
	}

	void* D12FrameBuffer::getColorAttachmentHandle() const {
		return reinterpret_cast<void*>(m_context->getSrvHeapWrapper().getGpuHandle(m_srvHeapIndex).ptr);
	}

}
