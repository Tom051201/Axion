#include "axpch.h"
#include "DX12Context.h"

#include "AxionEngine/Source/EngineConfig.h"
#include "AxionEngine/Source/core/EngineAssets.h"
#include "AxionEngine/Source/graphics/SwapChainSpecification.h"
#include "AxionEngine/Source/graphics/Renderer.h"
#include "AxionEngine/Source/graphics/Formats.h"
#include "AxionEngine/Source/graphics/Buffers.h"
#include "AxionEngine/Source/graphics/RenderContext.h"

#include "AxionEngine/Platform/windows/WindowsHelper.h"

#ifdef AX_DEBUG
#include "AxionEngine/Platform/directx12/DX12DebugLayer.h"
#endif

namespace Axion {

	DX12Context::~DX12Context() {}

	void DX12Context::initialize(void* hwnd, uint32_t width, uint32_t height) {
		AX_CORE_ASSERT(hwnd, "HWND cannot be null");
		m_width = width;
		m_height = height;

		// ----- Enable D3D12 Debug layer -----
		#ifdef AX_DEBUG
		DX12DebugLayer::initialize();
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
		m_rtvHeap.initialize(m_device.getDevice(), Config::DX12_MaxRtvDescriptors);
		m_gpuSrvHeap.initialize(m_device.getDevice(), Config::DX12_MaxSrvDescriptors, true);
		m_gpuSrvHeap.reserve(Config::DX12_SrvHeapReserve);
		m_stagingSrvHeap.initialize(m_device.getDevice(), Config::DX12_MaxSrvDescriptors, false);
		m_dsvHeap.initialize(m_device.getDevice(), Config::DX12_MaxDsvDescriptors);
		m_swapChain.initialize((HWND)hwnd, m_device.getFactory(), m_commandQueue.getCommandQueue(), swapSpec);
		m_commandManager.initialize(m_device.getDevice(), swapSpec.bufferCount, 16);
		m_fence.initialize(m_device.getDevice());

		m_frameFenceValues.resize(swapSpec.bufferCount, 0);
		m_currentFenceValue = 1;
		m_currentFrameIndex = m_swapChain.getSwapChain()->GetCurrentBackBufferIndex();

		AX_CORE_LOG_INFO("Using gpu adapter: {0}", m_device.getAdapterName());
		AX_CORE_LOG_INFO("DirectX12 backend initialized successfully");
	}

	void DX12Context::shutdown() {
		waitForPreviousFrame();

		m_dsvHeap.release();
		m_stagingSrvHeap.release();
		m_gpuSrvHeap.release();
		m_fence.release();
		m_commandManager.release();
		m_swapChain.release();
		m_rtvHeap.release();
		m_commandQueue.release();
		m_device.release();

		#ifdef AX_DEBUG
		DX12DebugLayer::reportLiveObjects();
		#endif

		AX_CORE_LOG_INFO("DirectX12 backend shutdown");
	}

	void DX12Context::prepareRendering() {
		m_fence.wait(m_frameFenceValues[m_currentFrameIndex]);
		m_commandManager.beginFrame(m_currentFrameIndex);
		m_mainThreadContext = m_commandManager.acquireContext(0);

		auto* cmd = m_mainThreadContext->getCmdList();

		// ----- Transition barrier -----
		CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(
			m_swapChain.getBackBuffer(m_swapChain.getFrameIndex()),
			D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET
		);
		cmd->ResourceBarrier(1, &barrier);

		// ----- Set viewport and scissor -----
		CD3DX12_VIEWPORT viewport(0.0f, 0.0f, static_cast<float>(m_width), static_cast<float>(m_height));
		CD3DX12_RECT scissor(0, 0, m_width, m_height);
		getCommandList()->RSSetViewports(1, &viewport);
		getCommandList()->RSSetScissorRects(1, &scissor);

		ID3D12DescriptorHeap* heaps[] = { m_gpuSrvHeap.getHeap() };
		cmd->SetDescriptorHeaps(1, heaps);
	}

	void DX12Context::finishRendering() {
		auto* cmd = m_mainThreadContext->getCmdList();

		// ----- Reverse barrier -----
		CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(
			m_swapChain.getBackBuffer(m_currentFrameIndex),
			D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT
		);
		cmd->ResourceBarrier(1, &barrier);

		std::vector<ID3D12CommandList*> executionLists;
		std::vector<ID3D12CommandList*> workerLists = m_commandManager.getActiveCommandLists();
		executionLists.insert(executionLists.end(), workerLists.begin(), workerLists.end());

		m_mainThreadContext->end();
		executionLists.push_back(static_cast<ID3D12GraphicsCommandList*>(m_mainThreadContext->getNativeCommandList()));
		m_commandQueue.getCommandQueue()->ExecuteCommandLists(static_cast<UINT>(executionLists.size()), executionLists.data());

		m_swapChain.present(m_vsyncInterval, 0);

		m_frameFenceValues[m_currentFrameIndex] = m_currentFenceValue;
		m_fence.signal(m_commandQueue.getCommandQueue(), m_currentFenceValue);
		m_currentFenceValue++;

		m_currentFrameIndex = m_swapChain.getSwapChain()->GetCurrentBackBufferIndex();
		m_swapChain.setFrameIndex(m_currentFrameIndex);
		m_gpuSrvHeap.nextFrame();
	}

	void DX12Context::setClearColor(const Vec4& color) {
		m_clearColor = color;
	}

	void DX12Context::clear() {
		const float clearColor[] = { m_clearColor.x, m_clearColor.y, m_clearColor.z, m_clearColor.w };
		m_swapChain.clear(clearColor);
	}

	void DX12Context::waitForPreviousFrame() {
		m_fence.signal(m_commandQueue.getCommandQueue(), m_currentFenceValue);
		m_fence.wait(m_currentFenceValue);
		m_currentFenceValue++;
		m_currentFrameIndex = m_swapChain.getSwapChain()->GetCurrentBackBufferIndex();
		m_swapChain.setFrameIndex(m_currentFrameIndex);
	}

	void DX12Context::bindSwapChainRenderTarget() {
		m_swapChain.setAsRenderTarget();
	}

	void DX12Context::bindDepthOnlyRenderTarget(RenderContext* renderContext, const Ref<Texture2D>& depthTexture) {
		auto* depthTex = static_cast<DX12DepthTexture*>(depthTexture.get());
		auto cmdList = static_cast<ID3D12GraphicsCommandList*>(renderContext->getNativeCommandList());

		auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(
			depthTex->getResource(),
			D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_DEPTH_WRITE
		);
		cmdList->ResourceBarrier(1, &barrier);

		auto dsvHandle = depthTex->getDsvHandle();
		cmdList->OMSetRenderTargets(0, nullptr, FALSE, &dsvHandle);

		cmdList->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

		D3D12_VIEWPORT vp{ 0.0f, 0.0f, (float)depthTexture->getWidth(), (float)depthTexture->getHeight(), 0.0f, 1.0f };
		D3D12_RECT sc{ 0, 0, (LONG)depthTexture->getWidth(), (LONG)depthTexture->getHeight() };
		cmdList->RSSetViewports(1, &vp);
		cmdList->RSSetScissorRects(1, &sc);
	}

	void DX12Context::unbindDepthOnlyRenderTarget(RenderContext* renderContext, const Ref<Texture2D>& depthTexture) {
		auto* depthTex = static_cast<DX12DepthTexture*>(depthTexture.get());
		auto cmdList = static_cast<ID3D12GraphicsCommandList*>(renderContext->getNativeCommandList());

		auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(
			depthTex->getResource(), D3D12_RESOURCE_STATE_DEPTH_WRITE,
			D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE
		);
		cmdList->ResourceBarrier(1, &barrier);
	}

	void DX12Context::bindSrvTable(RenderContext* renderContext, uint32_t rootIndex, const std::array<Ref<Texture2D>, 16>& textures, uint32_t count) {
		auto* device = m_device.getDevice();

		uint32_t tableSize = Config::DX12_MaxTextureSlots;
		uint32_t batchStartOffset = m_gpuSrvHeap.allocateRange(tableSize);

		for (uint32_t i = 0; i < tableSize; i++) {
			Ref<Texture2D> tex = (i < count && textures[i]) ? textures[i] : nullptr;

			if (!tex && textures[0]) tex = textures[0];
			if (!tex) tex = EngineAssets::getWhiteTexture();

			auto srcHandle = m_stagingSrvHeap.getCpuHandle(tex->getSrvHeapIndex());
			auto destHandle = m_gpuSrvHeap.getCpuHandle(batchStartOffset + i);

			device->CopyDescriptorsSimple(1, destHandle, srcHandle, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		}

		auto gpuHandle = m_gpuSrvHeap.getGpuHandle(batchStartOffset);

		auto* cmdList = static_cast<ID3D12GraphicsCommandList*>(renderContext->getNativeCommandList());
		cmdList->SetGraphicsRootDescriptorTable(rootIndex, gpuHandle);
	}

	void DX12Context::resize(uint32_t width, uint32_t height) {
		if (width <= 0 || height <= 0) return;

		waitForPreviousFrame();

		m_width = width;
		m_height = height;

		m_swapChain.resize(width, height);
		m_currentFrameIndex = m_swapChain.getSwapChain()->GetCurrentBackBufferIndex();
	}

	void DX12Context::drawIndexed(RenderContext* renderContext, const Ref<VertexBuffer>& vb, const Ref<IndexBuffer>& ib, uint32_t instanceCount) {
		auto cmdList = static_cast<ID3D12GraphicsCommandList*>(renderContext->getNativeCommandList());
		cmdList->DrawIndexedInstanced(ib->getIndexCount(), instanceCount, 0, 0, 0);
	}

	void DX12Context::drawIndexed(RenderContext* renderContext, const Ref<IndexBuffer>& ib, uint32_t indexCount, uint32_t instanceCount, uint32_t startIndexLocation, int32_t baseVertexLocation) {
		auto cmdList = static_cast<ID3D12GraphicsCommandList*>(renderContext->getNativeCommandList());
		cmdList->DrawIndexedInstanced(indexCount, instanceCount, startIndexLocation, baseVertexLocation, 0);
	}

	void DX12Context::draw(RenderContext* renderContext, uint32_t vertexCount) {
		auto cmdList = static_cast<ID3D12GraphicsCommandList*>(renderContext->getNativeCommandList());
		cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_LINELIST);
		cmdList->DrawInstanced(vertexCount, 1, 0, 0);
		cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	}

	std::string DX12Context::getGpuName() const {
		DXGI_ADAPTER_DESC1 desc;
		m_device.getAdapter()->GetDesc1(&desc);
		std::wstring ws(desc.Description);
		return WindowsHelper::WStringToString(ws);
	}

	std::string DX12Context::getGpuDriverVersion() const {
		LARGE_INTEGER driverVersion;
		if (SUCCEEDED(m_device.getAdapter()->CheckInterfaceSupport(__uuidof(IDXGIDevice), &driverVersion))) {
			WORD product = HIWORD(driverVersion.HighPart);
			WORD version = LOWORD(driverVersion.HighPart);
			WORD subVersion = HIWORD(driverVersion.LowPart);
			WORD build = LOWORD(driverVersion.LowPart);

			return std::to_string(product) + "." + std::to_string(version) + "." + std::to_string(subVersion) + "." + std::to_string(build);
		}
		else {
			return "Unknown";
		}
	}

	uint64_t DX12Context::getVramMB() const {
		DXGI_ADAPTER_DESC1 desc;
		m_device.getAdapter()->GetDesc1(&desc);
		return desc.DedicatedVideoMemory / (1024 * 1024);
	}

	void DX12Context::executeImmediateCommand(const std::function<void(ID3D12GraphicsCommandList*)>& command) {
		auto* device = m_device.getDevice();
		auto* queue = m_commandQueue.getCommandQueue();

		Microsoft::WRL::ComPtr<ID3D12CommandAllocator> allocator;
		AX_THROW_IF_FAILED_HR(device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&allocator)), "Failed to create temp allocator");

		Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> cmdList;
		AX_THROW_IF_FAILED_HR(device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, allocator.Get(), nullptr, IID_PPV_ARGS(&cmdList)), "Failed to create temp cmdList");

		command(cmdList.Get());

		cmdList->Close();
		ID3D12CommandList* ppCommandLists[] = { cmdList.Get() };
		queue->ExecuteCommandLists(1, ppCommandLists);

		Microsoft::WRL::ComPtr<ID3D12Fence> tempFence;
		AX_THROW_IF_FAILED_HR(device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&tempFence)), "Failed to create temporary fence");

		queue->Signal(tempFence.Get(), 1);

		if (tempFence->GetCompletedValue() < 1) {
			HANDLE eventHandle = CreateEvent(nullptr, FALSE, FALSE, nullptr);
			tempFence->SetEventOnCompletion(1, eventHandle);
			WaitForSingleObject(eventHandle, INFINITE);
			CloseHandle(eventHandle);
		}
	}

	RenderContext* DX12Context::acquireThreadContext(uint32_t threadIndex) {
		RenderContext* context = nullptr;

		if (threadIndex == 0) {
			context = m_mainThreadContext;
		}
		else {
			context = m_commandManager.acquireContext(threadIndex);
			auto* cmdList = static_cast<ID3D12GraphicsCommandList*>(context->getNativeCommandList());
			ID3D12DescriptorHeap* descriptorHeaps[] = { getSrvHeapWrapper().getHeap() };
			cmdList->SetDescriptorHeaps(1, descriptorHeaps);
		}

		return context;
	}

}
