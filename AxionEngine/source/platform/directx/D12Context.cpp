#include "axpch.h"
#include "D12Context.h"

#ifdef AX_DEBUG
	#include "D12DebugLayer.h"
#endif

namespace Axion {

	D12Context::~D12Context() {}

	void D12Context::initialize(void* hwnd, uint32_t width, uint32_t height) {
		m_width = width;
		m_height = height;

		#ifdef AX_DEBUG
		D12DebugLayer::initialize();
		#endif
		
		m_device.initialize();
		m_commandQueue.initialize(m_device.getDevice());
		m_swapChain.initialize((HWND)hwnd, m_device.getFactory(), m_commandQueue.getCommandQueue(), width, height);
		m_rtv.initialize(m_device.getDevice(), m_swapChain.getSwapChain());
		m_commandList.initialize(m_device.getDevice());
		m_fence.initialize(m_device.getDevice());

		AX_CORE_LOG_INFO("Using gpu adapter: {0}", m_device.getAdapterName());
		AX_CORE_LOG_INFO("DirectX12 backend initialized successfully");
	}



	void D12Context::shutdown() {

		waitForPreviousFrame();
		m_commandQueue.getCommandQueue()->Signal(m_fence.getFence(), m_fence.getFenceValue());

		m_fence.release();
		m_commandList.release();
		m_rtv.release();
		m_swapChain.release();
		m_commandQueue.release();
		m_device.release();

		#ifdef AX_DEBUG
		D12DebugLayer::reportLiveObjects();
		#endif

		AX_CORE_LOG_INFO("DirectX12 backend shutdown");
	}



	void D12Context::present() {

		m_commandQueue.executeCommandList(m_commandList.getCommandList());

		m_swapChain.present(m_vsyncInterval, 0);

		waitForPreviousFrame();
	}



	void D12Context::clear(float r, float g, float b, float a) {
		const float clearColor[] = { r, g, b, a };

		D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = m_rtv.getRTVHeap()->GetCPUDescriptorHandleForHeapStart();
		rtvHandle.ptr += m_swapChain.getFrameIndex() * m_rtv.getRTVDescriptorSize();

		getCommandList()->OMSetRenderTargets(1, &rtvHandle, FALSE, nullptr);
		getCommandList()->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
	}



	void D12Context::beginFrame() {
		auto* cmd = m_commandList.getCommandList();

		AX_THROW_IF_FAILED_HR(m_commandList.getCommandAllocator()->Reset(), "Failed to reset command allocator");
		AX_THROW_IF_FAILED_HR(cmd->Reset(m_commandList.getCommandAllocator(), nullptr), "Failed to reset command list");

		cmd->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
			m_rtv.getRenderTarget(m_swapChain.getFrameIndex()),
			D3D12_RESOURCE_STATE_PRESENT,
			D3D12_RESOURCE_STATE_RENDER_TARGET
		));

		CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(
			m_rtv.getRTVHeap()->GetCPUDescriptorHandleForHeapStart(),
			static_cast<INT>(m_swapChain.getFrameIndex()),
			m_rtv.getRTVDescriptorSize()
		);

		cmd->OMSetRenderTargets(1, &rtvHandle, FALSE, nullptr);
		const float clearColor[] = { 0.0f, 0.2f, 0.4f, 1.0f };
		cmd->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);

		// Set viewport and scissor
		CD3DX12_VIEWPORT viewport(0.0f, 0.0f, static_cast<float>(m_width), static_cast<float>(m_height));
		getCommandList()->RSSetViewports(1, &viewport);
		CD3DX12_RECT scissor(0, 0, m_width, m_height);
		getCommandList()->RSSetScissorRects(1, &scissor);
	}



	void D12Context::endFrame() {

		// reverse barrier
		m_commandList.getCommandList()->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
			m_rtv.getRenderTarget(m_swapChain.getFrameIndex()),
			D3D12_RESOURCE_STATE_RENDER_TARGET,
			D3D12_RESOURCE_STATE_PRESENT
		));

		m_commandList.close();
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
		if (width <= 0 || height <= 0)return;

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
