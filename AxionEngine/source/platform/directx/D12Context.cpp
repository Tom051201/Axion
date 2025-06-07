#include "axpch.h"
#include "D12Context.h"

namespace Axion {

	D12Context::~D12Context() {
		shutdown();
	}

	void D12Context::initialize(void* hwnd, uint32_t width, uint32_t height) {
		m_width = width;
		m_height = height;

#if defined(_DEBUG)
		Microsoft::WRL::ComPtr<ID3D12Debug> debugController;
		if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)))) {
			debugController->EnableDebugLayer();
			AX_CORE_LOG_INFO("D3D12 Debug Layer enabled");
		}
		else {
			AX_CORE_LOG_WARN("D3D12 Debug Layer not available");
		}
#endif
#if defined(_DEBUG)
		Microsoft::WRL::ComPtr<IDXGIInfoQueue> dxgiInfoQueue;
		if (SUCCEEDED(DXGIGetDebugInterface1(0, IID_PPV_ARGS(&dxgiInfoQueue)))) {
			dxgiInfoQueue->SetBreakOnSeverity(DXGI_DEBUG_ALL, DXGI_INFO_QUEUE_MESSAGE_SEVERITY_ERROR, TRUE);
			AX_CORE_LOG_INFO("DXGI Debug Layer enabled");
		}
		else {
			AX_CORE_LOG_WARN("DXGI Debug Layer not available");
		}
#endif

		m_device.initialize();
		m_commandQueue.initialize(m_device.getDevice());
		m_swapChain.initialize((HWND)hwnd, m_device.getFactory(), m_commandQueue.getCommandQueue(), width, height);
		m_srvHeap.initialize(m_device.getDevice(), 1024);
		m_rtv.initialize(m_device.getDevice(), m_swapChain.getSwapChain());
		m_commandList.initialize(m_device.getDevice());
		m_fence.initialize(m_device.getDevice());
	}



	void D12Context::shutdown() {
		#ifdef AX_DEBUG
		if (SUCCEEDED(DXGIGetDebugInterface1(0, IID_PPV_ARGS(&m_dxgiDebug)))) {
			m_dxgiDebug->ReportLiveObjects(DXGI_DEBUG_ALL, (DXGI_DEBUG_RLO_FLAGS)(DXGI_DEBUG_RLO_DETAIL | DXGI_DEBUG_RLO_IGNORE_INTERNAL));
		}
		#endif

		waitForPreviousFrame();
		m_commandQueue.getCommandQueue()->Signal(m_fence.getFence(), m_fence.getFenceValue());
		CloseHandle(m_fence.getFenceEvent());

		AX_CORE_LOG_INFO("D12Context shutdown");
	}



	void D12Context::present() {

		ID3D12CommandList* lists[] = { m_commandList.getCommandList() };
		m_commandQueue.getCommandQueue()->ExecuteCommandLists(1, lists);

		AX_THROW_IF_FAILED_HR(m_swapChain.getSwapChain()->Present(1, 0), "Failed to present swap chain");

		waitForPreviousFrame();
	}



	void D12Context::clear(float r, float g, float b, float a) {
		const float clearColor[] = { r, g, b, a };

		D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = m_rtv.getRTVHeap()->GetCPUDescriptorHandleForHeapStart();
		rtvHandle.ptr += m_swapChain.getFrameIndex() * m_rtv.getRTVDescriptorSize();

		m_commandList.getCommandList()->OMSetRenderTargets(1, &rtvHandle, FALSE, nullptr);
		m_commandList.getCommandList()->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
	}



	void D12Context::beginFrame() {
		AX_THROW_IF_FAILED_HR(m_commandList.getCommandAllocator()->Reset(), "Failed to reset command allocator");
		AX_THROW_IF_FAILED_HR(m_commandList.getCommandList()->Reset(m_commandList.getCommandAllocator(), nullptr), "Failed to reset command list");

		D3D12_RESOURCE_BARRIER barrier = {};
		barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		barrier.Transition.pResource = m_rtv.getRenderTarget(m_swapChain.getFrameIndex());
		barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
		barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
		barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
		m_commandList.getCommandList()->ResourceBarrier(1, &barrier);

		D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = m_rtv.getRTVHeap()->GetCPUDescriptorHandleForHeapStart();
		rtvHandle.ptr += m_swapChain.getFrameIndex() * m_rtv.getRTVDescriptorSize();

		m_commandList.getCommandList()->OMSetRenderTargets(1, &rtvHandle, FALSE, nullptr);
		const float clearColor[] = { 0.0f, 0.2f, 0.4f, 1.0f };
		m_commandList.getCommandList()->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);

		// Set viewport and scissor
		D3D12_VIEWPORT viewport = {};
		viewport.TopLeftX = 0;
		viewport.TopLeftY = 0;
		viewport.Width = static_cast<float>(m_width);
		viewport.Height = static_cast<float>(m_height);
		viewport.MinDepth = 0.0f;
		viewport.MaxDepth = 1.0f;
		m_commandList.getCommandList()->RSSetViewports(1, &viewport);

		D3D12_RECT scissorRect = {};
		scissorRect.left = 0;
		scissorRect.top = 0;
		scissorRect.right = m_width;
		scissorRect.bottom = m_height;
		m_commandList.getCommandList()->RSSetScissorRects(1, &scissorRect);
	}



	void D12Context::endFrame() {

		// Reverse barrier
		D3D12_RESOURCE_BARRIER barrier = {};
		barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		barrier.Transition.pResource = m_rtv.getRenderTarget(m_swapChain.getFrameIndex());
		barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
		barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
		barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
		m_commandList.getCommandList()->ResourceBarrier(1, &barrier);

		AX_THROW_IF_FAILED_HR(m_commandList.getCommandList()->Close(), "Failed to close the command list");
	}



	void D12Context::waitForPreviousFrame() {
		const UINT64 currentFence = m_fence.getFenceValue();
		AX_THROW_IF_FAILED_HR(m_commandQueue.getCommandQueue()->Signal(m_fence.getFence(), currentFence), "Failed to signal fence");
		m_fence.incrFenceValue();

		if (m_fence.getFence()->GetCompletedValue() < currentFence) {
			AX_THROW_IF_FAILED_HR(m_fence.getFence()->SetEventOnCompletion(currentFence, m_fence.getFenceEvent()), "Failed to set fence event");
			WaitForSingleObject(m_fence.getFenceEvent(), INFINITE);
		}

		m_swapChain.setFrameIndex(m_swapChain.getSwapChain()->GetCurrentBackBufferIndex());
	}



	void D12Context::resize(uint32_t width, uint32_t height) {
		if (width == 0 || height == 0)return;

		waitForPreviousFrame();

		m_width = width;
		m_height = height;

		m_rtv.resetRTVs();

		std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>> targets;
		for (int i = 0; i < AX_MAX_SWAPCHAIN_BUFFERS; i++) {
			targets.push_back(m_rtv.getRenderTarget(i));
		}

		m_swapChain.resize(
			width,
			height,
			m_device.getDevice(),
			m_rtv.getRTVHeap()->GetCPUDescriptorHandleForHeapStart(),
			m_rtv.getRTVDescriptorSize(),
			targets
		);
		
		m_rtv.initialize(m_device.getDevice(), m_swapChain.getSwapChain());

	}

}
