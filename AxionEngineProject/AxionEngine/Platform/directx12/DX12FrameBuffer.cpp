#include "axpch.h"
#include "DX12FrameBuffer.h"

#include "AxionEngine/Platform/directx12/DX12Context.h"
#include "AxionEngine/Platform/directx12/DX12Helpers.h"

namespace Axion {

	DX12FrameBuffer::DX12FrameBuffer(const FrameBufferSpecification& spec) : m_specification(spec) {
		m_context = static_cast<DX12Context*>(GraphicsContext::get()->getNativeContext());
		AX_CORE_ASSERT(m_context, "Failed to acquire DirectX12 context");
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

	DX12FrameBuffer::~DX12FrameBuffer() {
		release();
	}

	void DX12FrameBuffer::release() {
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

		if (m_entityIdResource) {
			m_entityIdResource.Reset();
			m_readbackBuffer.Reset();
			m_context->getRtvHeapWrapper().free(m_entityIdRtvHeapIndex);
		}
	}

	void DX12FrameBuffer::resize(uint32_t width, uint32_t height) {

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
		texDesc.Format = DX12Helpers::toDX12ColorFormat(m_specification.textureFormat);
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


		// ----- Optional Entity ID Attachment -----
		if (m_specification.useEntityIDAttachment) {
			m_entityIdRtvHeapIndex = m_context->getRtvHeapWrapper().allocate();
			m_entityIdState = D3D12_RESOURCE_STATE_RENDER_TARGET;

			D3D12_RESOURCE_DESC idDesc = texDesc;
			idDesc.Format = DXGI_FORMAT_R32_SINT;

			D3D12_CLEAR_VALUE idClear = {};
			idClear.Format = idDesc.Format;
			int clearID = -1;
			idClear.Color[0] = reinterpret_cast<float&>(clearID);
			idClear.Color[1] = 0.0f;
			idClear.Color[2] = 0.0f;
			idClear.Color[3] = 0.0f;

			hr = device->CreateCommittedResource(
				&heapProps, D3D12_HEAP_FLAG_NONE, &idDesc,
				m_entityIdState, &idClear, IID_PPV_ARGS(&m_entityIdResource)
			);
			AX_THROW_IF_FAILED_HR(hr, "Failed to create Entity ID resource");

			// -- Create RTV --
			D3D12_RENDER_TARGET_VIEW_DESC idRtvDesc = {};
			idRtvDesc.Format = idDesc.Format;
			idRtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
			device->CreateRenderTargetView(m_entityIdResource.Get(), &idRtvDesc, m_context->getRtvHeapWrapper().getCpuHandle(m_entityIdRtvHeapIndex));

			// -- Create 1-Pixel Readback Buffer --
			CD3DX12_HEAP_PROPERTIES readbackHeapProps(D3D12_HEAP_TYPE_READBACK);
			D3D12_RESOURCE_DESC readbackDesc = CD3DX12_RESOURCE_DESC::Buffer(sizeof(int));
			device->CreateCommittedResource(
				&readbackHeapProps, D3D12_HEAP_FLAG_NONE, &readbackDesc,
				D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(&m_readbackBuffer)
			);
		}


		// ----- Depth -----
		DXGI_FORMAT depthFormat = DX12Helpers::toDX12DepthStencilFormat(m_specification.depthStencilFormat);
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

	void DX12FrameBuffer::bind() const {
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
		D3D12_CPU_DESCRIPTOR_HANDLE rtvHandles[2];
		rtvHandles[0] = m_context->getRtvHeapWrapper().getCpuHandle(m_rtvHeapIndex);
		uint32_t numTargets = 1;

		if (m_specification.useEntityIDAttachment) {
			rtvHandles[1] = m_context->getRtvHeapWrapper().getCpuHandle(m_entityIdRtvHeapIndex);
			numTargets = 2;
		}

		auto dsvHandle = m_context->getDsvHeapWrapper().getCpuHandle(m_dsvHeapIndex);
		cmdList->OMSetRenderTargets(numTargets, rtvHandles, FALSE, &dsvHandle);


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

	void DX12FrameBuffer::unbind() const {
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
	void DX12FrameBuffer::clear(const Vec4& clearColor) {
		auto* cmdList = m_context->getCommandList();
		auto rtvHandle = m_context->getRtvHeapWrapper().getCpuHandle(m_rtvHeapIndex);
		auto dsvHandle = m_context->getDsvHeapWrapper().getCpuHandle(m_dsvHeapIndex);

		#ifdef AX_DEBUG
		if (clearColor != m_specification.clearColor) {
			AX_CORE_LOG_WARN("Clearing the DX12FrameBuffer with not optimized color!");
		}
		#endif

		// -- Clear Color Attachment --
		float color[] = { clearColor.x, clearColor.y, clearColor.z, clearColor.w };
		cmdList->ClearRenderTargetView(rtvHandle, color, 0, nullptr);
		cmdList->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

		// -- Clear Entity ID Attachment --
		if (m_specification.useEntityIDAttachment) {
			auto idHandle = m_context->getRtvHeapWrapper().getCpuHandle(m_entityIdRtvHeapIndex);
			int clearID = -1;
			float idClearColor[4] = { reinterpret_cast<float&>(clearID), 0.0f, 0.0f, 0.0f };
			cmdList->ClearRenderTargetView(idHandle, idClearColor, 0, nullptr);
		}
	}

	void DX12FrameBuffer::clear() {
		clear(m_specification.clearColor);
	}

	void DX12FrameBuffer::clearDepth() {
		auto* cmdList = m_context->getCommandList();
		auto dsvHandle = m_context->getDsvHeapWrapper().getCpuHandle(m_dsvHeapIndex);

		cmdList->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
	}

	void* DX12FrameBuffer::getColorAttachmentHandle() const {
		auto* device = m_context->getDevice();

		uint32_t viewIndex = m_context->getSrvHeapWrapper().allocate();

		auto destHandle = m_context->getSrvHeapWrapper().getCpuHandle(viewIndex);
		auto srcHandle = m_context->getStagingSrvHeapWrapper().getCpuHandle(m_srvHeapIndex);

		device->CopyDescriptorsSimple(1, destHandle, srcHandle, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

		return reinterpret_cast<void*>(m_context->getSrvHeapWrapper().getGpuHandle(viewIndex).ptr);
	}

	void DX12FrameBuffer::clearAttachment(uint32_t attachmentIndex, int value) {
		if (attachmentIndex == 1 && m_specification.useEntityIDAttachment) {
			auto* cmdList = m_context->getCommandList();
			auto rtvHandle = m_context->getRtvHeapWrapper().getCpuHandle(m_entityIdRtvHeapIndex);

			float clearVal = reinterpret_cast<float&>(value);
			float clearColor[4] = { clearVal, 0.0f, 0.0f, 0.0f };

			cmdList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
		}
	}

	int DX12FrameBuffer::readPixel(uint32_t attachmentIndex, int x, int y) {
		if (attachmentIndex != 1 || !m_specification.useEntityIDAttachment) return -1;

		// 1. Read the pixel data from the PREVIOUS frame (100% safe, no stalls required!)
		int entityID = -1;
		int* mappedData;
		if (SUCCEEDED(m_readbackBuffer->Map(0, nullptr, reinterpret_cast<void**>(&mappedData)))) {
			entityID = mappedData[0];
			m_readbackBuffer->Unmap(0, nullptr);
		}

		// 2. Queue a new copy command for THIS frame
		if (x >= 0 && y >= 0 && x < (int)m_specification.width && y < (int)m_specification.height) {
			auto* cmdList = m_context->getCommandList();

			auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(m_entityIdResource.Get(), m_entityIdState, D3D12_RESOURCE_STATE_COPY_SOURCE);
			cmdList->ResourceBarrier(1, &barrier);

			D3D12_PLACED_SUBRESOURCE_FOOTPRINT footprint = {};
			footprint.Offset = 0;
			footprint.Footprint.Format = DXGI_FORMAT_R32_SINT;
			footprint.Footprint.Width = 1;
			footprint.Footprint.Height = 1;
			footprint.Footprint.Depth = 1;
			footprint.Footprint.RowPitch = 256;

			CD3DX12_TEXTURE_COPY_LOCATION dst(m_readbackBuffer.Get(), footprint);
			CD3DX12_TEXTURE_COPY_LOCATION src(m_entityIdResource.Get(), 0);
			D3D12_BOX box = { (UINT)x, (UINT)y, 0, (UINT)x + 1, (UINT)y + 1, 1 };

			cmdList->CopyTextureRegion(&dst, 0, 0, 0, &src, &box);

			auto barrierBack = CD3DX12_RESOURCE_BARRIER::Transition(m_entityIdResource.Get(), D3D12_RESOURCE_STATE_COPY_SOURCE, m_entityIdState);
			cmdList->ResourceBarrier(1, &barrierBack);
		}

		return entityID;
	}

}
