#include "axpch.h"
#include "D12Context.h"

#ifdef AX_DEBUG
	#include "AxionEngine/Platform/directx/D12DebugLayer.h"
#endif

namespace Axion {

	D12Context::~D12Context() {}

	void D12Context::initialize(void* hwnd, uint32_t width, uint32_t height) {
		AX_ASSERT(hwnd, "HWND cannot be null");
		m_width = width;
		m_height = height;

		#ifdef AX_DEBUG
		D12DebugLayer::initialize();
		#endif
		
		m_device.initialize();
		m_commandQueue.initialize(m_device.getDevice());
		m_rtvHeap.initialize(m_device.getDevice(), AX_D12_MAX_RTV_DESCRIPTORS);
		m_swapChain.initialize((HWND)hwnd, m_device.getFactory(), m_commandQueue.getCommandQueue(), width, height);
		m_commandList.initialize(m_device.getDevice());
		m_fence.initialize(m_device.getDevice());
		m_srvHeap.initialize(m_device.getDevice(), AX_D12_MAX_SRV_DESCRIPTORS);

		AX_CORE_LOG_INFO("Using gpu adapter: {0}", m_device.getAdapterName());
		AX_CORE_LOG_INFO("DirectX12 backend initialized successfully");
	}

	void D12Context::shutdown() {

		waitForPreviousFrame();
		m_commandQueue.getCommandQueue()->Signal(m_fence.getFence(), m_fence.getFenceValue());

		m_srvHeap.release();
		m_fence.release();
		m_commandList.release();
		m_swapChain.release();
		m_rtvHeap.release();
		m_commandQueue.release();
		m_device.release();

		#ifdef AX_DEBUG
		D12DebugLayer::reportLiveObjects();
		#endif

		AX_CORE_LOG_INFO("DirectX12 backend shutdown");
	}

	void D12Context::prepareRendering() {
		auto* cmd = m_commandList.getCommandList();

		AX_THROW_IF_FAILED_HR(m_commandList.getCommandAllocator()->Reset(), "Failed to reset command allocator");
		AX_THROW_IF_FAILED_HR(cmd->Reset(m_commandList.getCommandAllocator(), nullptr), "Failed to reset command list");

		cmd->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
			m_swapChain.getBackBuffer(m_swapChain.getFrameIndex()),
			D3D12_RESOURCE_STATE_PRESENT,
			D3D12_RESOURCE_STATE_RENDER_TARGET
		));

		clear();

		// Set viewport and scissor
		CD3DX12_VIEWPORT viewport(0.0f, 0.0f, static_cast<float>(m_width), static_cast<float>(m_height));
		CD3DX12_RECT scissor(0, 0, m_width, m_height);
		getCommandList()->RSSetViewports(1, &viewport);
		getCommandList()->RSSetScissorRects(1, &scissor);
	}

	void D12Context::finishRendering() {
		// reverse barrier
		m_commandList.getCommandList()->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
			m_swapChain.getBackBuffer(m_swapChain.getFrameIndex()),
			D3D12_RESOURCE_STATE_RENDER_TARGET,
			D3D12_RESOURCE_STATE_PRESENT
		));

		m_commandList.close();

		m_commandQueue.executeCommandList(m_commandList.getCommandList());
		m_swapChain.present(m_vsyncInterval, 0);
		waitForPreviousFrame();
	}

	void D12Context::setClearColor(const Vec4& color) {
		m_clearColor = color;
	}

	void D12Context::clear() {
		const float clearColor[] = { m_clearColor.x, m_clearColor.y, m_clearColor.z, m_clearColor.w };

		D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = m_rtvHeap.getHeap()->GetCPUDescriptorHandleForHeapStart();
		rtvHandle.ptr += m_swapChain.getFrameIndex() * m_rtvHeap.getDescriptorSize();

		getCommandList()->OMSetRenderTargets(1, &rtvHandle, FALSE, nullptr);
		getCommandList()->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
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

		m_swapChain.resize(width, height);
	
	}

	std::string D12Context::getGpuName() const {
		DXGI_ADAPTER_DESC1 desc;
		m_device.getAdapter()->GetDesc1(&desc);
		std::wstring ws(desc.Description);
		return std::string(ws.begin(), ws.end());
	}

	std::string D12Context::getGpuDriverVersion() const {
		LARGE_INTEGER driverVersion;
		if (SUCCEEDED(m_device.getAdapter()->CheckInterfaceSupport(__uuidof(IDXGIDevice), &driverVersion))) {
			WORD product = HIWORD(driverVersion.HighPart);
			WORD version = LOWORD(driverVersion.HighPart);
			WORD subVersion = HIWORD(driverVersion.LowPart);
			WORD build = LOWORD(driverVersion.LowPart);

			return
				std::to_string(product) + "." +
				std::to_string(version) + "." +
				std::to_string(subVersion) + "." +
				std::to_string(build);
		}
		else {
			return "Unknown";
		}
	}

	uint64_t D12Context::getVramMB() const {
		DXGI_ADAPTER_DESC1 desc;
		m_device.getAdapter()->GetDesc1(&desc);
		return desc.DedicatedVideoMemory / (1024 * 1024);
	}

}
