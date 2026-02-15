#pragma once
#include "axpch.h"

#include "AxionEngine/Source/core/Math.h"
#include "AxionEngine/Source/render/GraphicsContext.h"

#include "AxionEngine/Platform/directx/D12Device.h"
#include "AxionEngine/Platform/directx/D12CommandQueue.h"
#include "AxionEngine/Platform/directx/D12SwapChain.h"
#include "AxionEngine/Platform/directx/D12CommandList.h"
#include "AxionEngine/Platform/directx/D12Fence.h"
#include "AxionEngine/Platform/directx/D12DescriptorHeaps.h"
#include "AxionEngine/Platform/directx/D12Texture.h"

namespace Axion {

	class D12Context : public GraphicsContext {
	public:

		~D12Context() override;

		void initialize(void* hwnd, uint32_t width, uint32_t height) override;
		void shutdown() override;
		void* getNativeContext() const override { return (void*)this; }

		void prepareRendering() override;
		void finishRendering() override;

		// ----- Clear swap chain -----
		void setClearColor(const Vec4& color) override;
		void clear() override;

		void bindSwapChainRenderTarget() override;
		void* getImGuiTextureID(const Ref<Texture2D>& texture) override;
		void bindSrvTable(uint32_t rootIndex, const std::array<Ref<Texture2D>, 16>& textures, uint32_t count);

		void resize(uint32_t width, uint32_t height) override;

		void activateVsync() override { m_vsyncInterval = 1; };
		void deactivateVsync() override { m_vsyncInterval = 0; };

		// ----- Draw Calls -----
		void drawIndexed(const Ref<VertexBuffer>& vb, const Ref<IndexBuffer>& ib) override;
		void drawIndexed(const Ref<IndexBuffer>& ib, uint32_t indexCount) override;

		// ----- Util functions -----
		std::string getGpuName() const override;
		std::string getGpuDriverVersion() const override;
		uint64_t getVramMB() const override;
		void waitForPreviousFrame();

		// ----- Getter for D3D12 components -----
		D12Device& getDeviceWrapper() { return m_device; }
		D12CommandQueue& getCommandQueueWrapper() { return m_commandQueue; }
		D12rtvHeap& getRtvHeapWrapper() { return m_rtvHeap; }
		D12SwapChain& getSwapChainWrapper() { return m_swapChain; }
		D12CommandList& getCommandListWrapper() { return m_commandList; }
		D12Fence& getFenceWrapper() { return m_fence; }
		D12srvHeap& getSrvHeapWrapper() { return m_gpuSrvHeap; }
		D12srvHeap& getStagingSrvHeapWrapper() { return m_stagingSrvHeap; }
		D12dsvHeap& getDsvHeapWrapper() { return m_dsvHeap; }

		ID3D12Device* getDevice() const { return m_device.getDevice(); }
		IDXGIFactory6* getFactory() const { return m_device.getFactory(); }
		IDXGIAdapter1* getAdapter() const { return m_device.getAdapter(); }
		ID3D12CommandQueue* getCommandQueue() const { return m_commandQueue.getCommandQueue(); }
		IDXGISwapChain3* getSwapChain() const { return m_swapChain.getSwapChain(); }
		ID3D12GraphicsCommandList* getCommandList() const { return m_commandList.getCommandList(); }
		ID3D12CommandAllocator* getCommandAllocator() const { return m_commandList.getCommandAllocator(); }
		ID3D12Fence* getFence() const { return m_fence.getFence(); }

	private:

		uint32_t m_width = 0, m_height = 0;
		uint32_t m_vsyncInterval = 0;

		Vec4 m_clearColor = Vec4::zero();

		D12Device m_device;
		D12CommandQueue m_commandQueue;
		D12rtvHeap m_rtvHeap;
		D12SwapChain m_swapChain;
		D12CommandList m_commandList;
		D12Fence m_fence;
		D12srvHeap m_gpuSrvHeap;
		D12srvHeap m_stagingSrvHeap;
		D12dsvHeap m_dsvHeap;

	};

}
