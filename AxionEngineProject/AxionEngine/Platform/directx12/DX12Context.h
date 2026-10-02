#pragma once

#include <string>
#include <array>
#include <cstdint>

#include <d3d12.h>
#include <dxgi1_6.h>

#include "AxionEngine/Source/core/Math.h"
#include "AxionEngine/Source/graphics/Texture.h"
#include "AxionEngine/Source/graphics/GraphicsContext.h"

#include "AxionEngine/Platform/directx12/DX12Device.h"
#include "AxionEngine/Platform/directx12/DX12CommandQueue.h"
#include "AxionEngine/Platform/directx12/DX12SwapChain.h"
#include "AxionEngine/Platform/directx12/DX12CommandManager.h"
#include "AxionEngine/Platform/directx12/DX12Fence.h"
#include "AxionEngine/Platform/directx12/DX12DescriptorHeaps.h"
#include "AxionEngine/Platform/directx12/DX12Texture.h"

namespace Axion {

	class RenderContext;

	class DX12Context : public GraphicsContext {
	public:

		~DX12Context() override;

		void initialize(void* hwnd, uint32_t width, uint32_t height) override;
		void shutdown() override;
		void resize(uint32_t width, uint32_t height) override;
		void* getNativeContext() const override { return (void*)this; }

		void prepareRendering() override;
		void finishRendering() override;

		void setClearColor(const Vec4& color) override;
		void clear() override;

		void bindSwapChainRenderTarget() override;
		void bindDepthOnlyRenderTarget(RenderContext* renderContext, const Ref<Texture2D>& depthTexture) override;
		void unbindDepthOnlyRenderTarget(RenderContext* renderContext, const Ref<Texture2D>& depthTexture) override;

		void activateVsync() override { m_vsyncInterval = 1; };
		void deactivateVsync() override { m_vsyncInterval = 0; };

		RenderContext* getMainRenderContext() override { return m_mainThreadContext; }
		RenderContext* acquireThreadContext(uint32_t threadIndex);

		void drawIndexed(RenderContext* renderContext, const Ref<VertexBuffer>& vb, const Ref<IndexBuffer>& ib, uint32_t instanceCount = 1) override;
		void drawIndexed(RenderContext* renderContext, const Ref<IndexBuffer>& ib, uint32_t indexCount, uint32_t instanceCount = 1, uint32_t startIndexLocation = 0, int32_t baseVertexLocation = 0) override;
		void draw(RenderContext* renderContext, uint32_t vertexCount) override;

		std::string getGpuName() const override;
		std::string getGpuDriverVersion() const override;
		uint64_t getVramMB() const override;

		// ----- Getter for D3D12 components -----
		D12Device& getDeviceWrapper() { return m_device; }
		DX12CommandQueue& getCommandQueueWrapper() { return m_commandQueue; }
		DX12rtvHeap& getRtvHeapWrapper() { return m_rtvHeap; }
		DX12SwapChain& getSwapChainWrapper() { return m_swapChain; }
		DX12Fence& getFenceWrapper() { return m_fence; }
		DX12srvHeap& getSrvHeapWrapper() { return m_gpuSrvHeap; }
		DX12srvHeap& getStagingSrvHeapWrapper() { return m_stagingSrvHeap; }
		DX12dsvHeap& getDsvHeapWrapper() { return m_dsvHeap; }

		ID3D12Device* getDevice() const { return m_device.getDevice(); }
		IDXGIFactory6* getFactory() const { return m_device.getFactory(); }
		IDXGIAdapter1* getAdapter() const { return m_device.getAdapter(); }
		ID3D12CommandQueue* getCommandQueue() const { return m_commandQueue.getCommandQueue(); }
		IDXGISwapChain3* getSwapChain() const { return m_swapChain.getSwapChain(); }
		ID3D12GraphicsCommandList* getCommandList() const { return m_mainThreadContext->getCmdList(); }
		DX12CommandManager& getCommandManager() { return m_commandManager; }
		ID3D12Fence* getFence() const { return m_fence.getFence(); }
		uint32_t getCurrentFrameIndex() const { return m_currentFrameIndex; }



		void bindSrvTable(RenderContext* renderContext, uint32_t rootIndex, const std::array<Ref<Texture2D>, 16>& textures, uint32_t count);
		void executeImmediateCommand(const std::function<void(ID3D12GraphicsCommandList*)>& command);
		void waitForPreviousFrame();

	private:

		uint32_t m_width = 0, m_height = 0;
		uint32_t m_vsyncInterval = 0;

		Vec4 m_clearColor = Vec4::zero();

		D12Device m_device;
		DX12CommandQueue m_commandQueue;
		DX12rtvHeap m_rtvHeap;
		DX12SwapChain m_swapChain;
		DX12CommandManager m_commandManager;
		DX12Fence m_fence;
		DX12srvHeap m_gpuSrvHeap;
		DX12srvHeap m_stagingSrvHeap;
		DX12dsvHeap m_dsvHeap;

		std::vector<UINT64> m_frameFenceValues;
		UINT64 m_currentFenceValue = 1;
		uint32_t m_currentFrameIndex = 0;

		DX12RenderContext* m_mainThreadContext = nullptr;

	};

}
