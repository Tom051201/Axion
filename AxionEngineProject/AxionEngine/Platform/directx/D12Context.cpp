#include "axpch.h"
#include "D12Context.h"

#include "AxionEngine/Source/render/SwapChainSpecification.h"

#include "AxionEngine/Platform/windows/WindowsHelper.h"


#ifdef AX_DEBUG
	#include "AxionEngine/Platform/directx/D12DebugLayer.h"
#endif

namespace Axion {

	D12Context::~D12Context() {}

	void D12Context::initialize(void* hwnd, uint32_t width, uint32_t height) {
		AX_CORE_ASSERT(hwnd, "HWND cannot be null");
		m_width = width;
		m_height = height;


		// ----- Enable D3D12 Debug layer -----
		#ifdef AX_DEBUG
		D12DebugLayer::initialize();
		#endif


		// ----- Set swap chain specification -----
		SwapChainSpecification swapSpec;
		swapSpec.width = width;
		swapSpec.height = height;
		swapSpec.backBufferFormat = ColorFormat::RGBA8;
		swapSpec.depthBufferFormat = DepthStencilFormat::DEPTH32F;


		// ----- Initialize D3D12 backend -----
		m_device.initialize();
		m_commandQueue.initialize(m_device.getDevice());
		m_rtvHeap.initialize(m_device.getDevice(), AX_D12_MAX_RTV_DESCRIPTORS);
		m_srvHeap.initialize(m_device.getDevice(), AX_D12_MAX_SRV_DESCRIPTORS);
		m_dsvHeap.initialize(m_device.getDevice(), AX_D12_MAX_DSV_DESCRIPTORS);
		m_swapChain.initialize((HWND)hwnd, m_device.getFactory(), m_commandQueue.getCommandQueue(), swapSpec);
		m_commandList.initialize(m_device.getDevice());
		m_fence.initialize(m_device.getDevice());

		AX_CORE_LOG_INFO("Using gpu adapter: {0}", m_device.getAdapterName());
		AX_CORE_LOG_INFO("DirectX12 backend initialized successfully");
	}

	void D12Context::shutdown() {

		waitForPreviousFrame();
		m_commandQueue.getCommandQueue()->Signal(m_fence.getFence(), m_fence.getFenceValue());

		m_dsvHeap.release();
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


		// ----- Transition barrier -----
		cmd->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
			m_swapChain.getBackBuffer(m_swapChain.getFrameIndex()),
			D3D12_RESOURCE_STATE_PRESENT,
			D3D12_RESOURCE_STATE_RENDER_TARGET
		));

		clear();


		// ----- Set viewport and scissor -----
		CD3DX12_VIEWPORT viewport(0.0f, 0.0f, static_cast<float>(m_width), static_cast<float>(m_height));
		CD3DX12_RECT scissor(0, 0, m_width, m_height);
		getCommandList()->RSSetViewports(1, &viewport);
		getCommandList()->RSSetScissorRects(1, &scissor);

		ID3D12DescriptorHeap* heaps[] = { m_srvHeap.getHeap() };
		cmd->SetDescriptorHeaps(1, heaps);
	}

	void D12Context::finishRendering() {
		// ----- Reverse barrier -----
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
		m_swapChain.clear(clearColor);
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

	void D12Context::bindSwapChainRenderTarget() {
		m_swapChain.setAsRenderTarget();
	}

	void D12Context::bindSrvTable(uint32_t rootIndex, const std::array<Ref<Texture2D>, 16>& textures, uint32_t count) {
		auto* device = m_device.getDevice();
		auto* cmdList = m_commandList.getCommandList();

		uint32_t batchStartOffset = m_srvHeap.allocateRange(16);
		D3D12_CPU_DESCRIPTOR_HANDLE destHandle = m_srvHeap.getCpuHandle(batchStartOffset);

		for (uint32_t i = 0; i < 16; i++) {
			D3D12_CPU_DESCRIPTOR_HANDLE srcHandle;
			if (i < count && textures[i]) {
				srcHandle = m_srvHeap.getCpuHandle(static_cast<D12Texture2D*>(textures[i].get())->getSrvHeapIndex());
			}
			else {
				srcHandle = m_srvHeap.getCpuHandle(static_cast<D12Texture2D*>(textures[0].get())->getSrvHeapIndex());
			}

			device->CopyDescriptorsSimple(1, destHandle, srcHandle, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
			destHandle.ptr += device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		}

		auto gpuHandle = m_srvHeap.getGpuHandle(batchStartOffset);
		cmdList->SetGraphicsRootDescriptorTable(rootIndex, gpuHandle);
	}

	void D12Context::resize(uint32_t width, uint32_t height) {
		if (width <= 0 || height <= 0) return;

		waitForPreviousFrame();

		m_width = width;
		m_height = height;

		m_swapChain.resize(width, height);
	}

	void D12Context::drawIndexed(const Ref<VertexBuffer>& vb, const Ref<IndexBuffer>& ib) {
		m_commandList.getCommandList()->DrawIndexedInstanced(ib->getIndexCount(), 1, 0, 0, 0);
	}

	void D12Context::drawIndexed(const Ref<IndexBuffer>& ib, uint32_t indexCount) {
		m_commandList.getCommandList()->DrawIndexedInstanced(indexCount, 1, 0, 0, 0);
	}

	std::string D12Context::getGpuName() const {
		DXGI_ADAPTER_DESC1 desc;
		m_device.getAdapter()->GetDesc1(&desc);
		std::wstring ws(desc.Description);
		return WindowsHelper::WStringToString(ws);
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
