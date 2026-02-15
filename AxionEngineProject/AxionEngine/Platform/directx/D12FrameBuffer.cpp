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
			AX_CORE_LOG_ERROR("Error creating frame buffer");
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
			m_context->getStagingSrvHeapWrapper().free(m_srvHeapIndex);
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


		// ----- Destroy old framebuffer -----
		release();


		// ----- Reallocate descriptor heap indices -----
		m_rtvHeapIndex = m_context->getRtvHeapWrapper().allocate();
		m_srvHeapIndex = m_context->getStagingSrvHeapWrapper().allocate();
		m_dsvHeapIndex = m_context->getDsvHeapWrapper().allocate();

		auto* device = m_context->getDevice();

		m_specification.width = width;
		m_specification.height = height;


		// ----- Texture -----
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

		// -- Set color clear value --
		D3D12_CLEAR_VALUE clearValue = {};
		clearValue.Format = texDesc.Format;
		clearValue.Color[0] = m_specification.clearColor.x;
		clearValue.Color[1] = m_specification.clearColor.y;
		clearValue.Color[2] = m_specification.clearColor.z;
		clearValue.Color[3] = m_specification.clearColor.w;

		// -- Create color resource --
		CD3DX12_HEAP_PROPERTIES heapProps(D3D12_HEAP_TYPE_DEFAULT);
		HRESULT hr = device->CreateCommittedResource(
			&heapProps,
			D3D12_HEAP_FLAG_NONE,
			&texDesc,
			m_currentState,
			&clearValue,
			IID_PPV_ARGS(&m_colorResource)
		);
		AX_THROW_IF_FAILED_HR(hr, "Failed to create frame buffer color resource");


		// ----- Depth -----
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

		// -- Set depth clear value --
		D3D12_CLEAR_VALUE depthClearValue = {};
		depthClearValue.Format = depthFormat;
		depthClearValue.DepthStencil.Depth = 1.0f;
		depthClearValue.DepthStencil.Stencil = 0;

		// -- Create depth resource --
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


		// ----- Depth Stencil View -----
		D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
		dsvDesc.Format = depthFormat;
		dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
		dsvDesc.Flags = D3D12_DSV_FLAG_NONE;

		device->CreateDepthStencilView(
			m_depthResource.Get(),
			&dsvDesc,
			m_context->getDsvHeapWrapper().getCpuHandle(m_dsvHeapIndex)
		);


		// ----- Render Target View (RTV) -----
		device->CreateRenderTargetView(m_colorResource.Get(), nullptr, m_context->getRtvHeapWrapper().getCpuHandle(m_rtvHeapIndex));


		// ----- Shader Resource View (SRV) -----
		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srvDesc.Format = texDesc.Format;
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MipLevels = 1;
		m_context->getDevice()->CreateShaderResourceView(m_colorResource.Get(), &srvDesc, m_context->getStagingSrvHeapWrapper().getCpuHandle(m_srvHeapIndex));


		#ifdef AX_DEBUG
		m_colorResource->SetName(L"FrameBufferColor");
		m_depthResource->SetName(L"FrameBufferDepth");
		#endif
	}

	void D12FrameBuffer::bind() const {
		auto* cmdList = m_context->getCommandList();
		

		// ----- Transition barrier -----
		if (m_currentState != D3D12_RESOURCE_STATE_RENDER_TARGET) {
			auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(
				m_colorResource.Get(),
				m_currentState,
				D3D12_RESOURCE_STATE_RENDER_TARGET
			);
			cmdList->ResourceBarrier(1, &barrier);
			m_currentState = D3D12_RESOURCE_STATE_RENDER_TARGET;
		}


		// ----- Set Render Target -----
		auto rtvHandle = m_context->getRtvHeapWrapper().getCpuHandle(m_rtvHeapIndex);
		auto dsvHandle = m_context->getDsvHeapWrapper().getCpuHandle(m_dsvHeapIndex);
		cmdList->OMSetRenderTargets(1, &rtvHandle, FALSE, &dsvHandle);


		// ----- Set viewport and scissor -----
		D3D12_VIEWPORT vp{};
		vp.TopLeftX = 0.0f;
		vp.TopLeftY = 0.0f;
		vp.Width = static_cast<float>(m_specification.width);
		vp.Height = static_cast<float>(m_specification.height);
		vp.MinDepth = 0.0f;
		vp.MaxDepth = 1.0f;
		cmdList->RSSetViewports(1, &vp);

		D3D12_RECT sc{};
		sc.left = 0;
		sc.top = 0;
		sc.right = static_cast<LONG>(m_specification.width);
		sc.bottom = static_cast<LONG>(m_specification.height);
		cmdList->RSSetScissorRects(1, &sc);
	}

	void D12FrameBuffer::unbind() const {
		auto* cmdList = m_context->getCommandList();

		// ----- Reverse barrier -----
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

	// NOTE:
	// Using different values here than specified
	// in the resource description causes warning!
	// Setting a color here does not change the
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
		auto* device = m_context->getDevice();

		uint32_t viewIndex = m_context->getSrvHeapWrapper().allocate();

		auto destHandle = m_context->getSrvHeapWrapper().getCpuHandle(viewIndex);
		auto srcHandle = m_context->getStagingSrvHeapWrapper().getCpuHandle(m_srvHeapIndex);

		device->CopyDescriptorsSimple(1, destHandle, srcHandle, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

		return reinterpret_cast<void*>(m_context->getSrvHeapWrapper().getGpuHandle(viewIndex).ptr);
	}

}
