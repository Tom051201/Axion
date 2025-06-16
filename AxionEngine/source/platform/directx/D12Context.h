#pragma once

#include "axpch.h"

#include "Axion/core/Math.h"
#include "Axion/render/GraphicsContext.h"

#include "platform/directx/D12Device.h"
#include "platform/directx/D12CommandQueue.h"
#include "platform/directx/D12SwapChain.h"
#include "platform/directx/D12RenderTarget.h"
#include "platform/directx/D12CommandList.h"
#include "platform/directx/D12Fence.h"
#include "platform/directx/D12srvHeap.h"

namespace Axion {

	class D12Context : public GraphicsContext {
	public:

		~D12Context() override;

		void initialize(void* hwnd, uint32_t width, uint32_t height) override;
		void shutdown() override;

		void present();
		void setClearColor(const Vec4& color);
		void clear();
		void beginFrame();
		void endFrame();

		void resize(uint32_t width, uint32_t height);

		void activateVsync() { m_vsyncInterval = 1; };
		void deactivateVsync() { m_vsyncInterval = 0; };

		inline void* getNativeContext() const override { return (void*)this; }
		
		void waitForPreviousFrame();

		D12Device& getDeviceWrapper() { return m_device; }
		D12CommandQueue& getCommandQueueWrapper() { return m_commandQueue; }
		D12SwapChain& getSwapChainWrapper() { return m_swapChain; }
		D12RenderTarget& getRtvWrapper() { return m_rtv; }
		D12CommandList& getCommandListWrapper() { return m_commandList; }
		D12Fence& getFenceWrapper() { return m_fence; }
		D12srvHeap& getSrvHeapWrapper() { return m_srvHeap; }

		ID3D12Device* getDevice() const { return m_device.getDevice(); }
		IDXGIFactory6* getFactory() const { return m_device.getFactory(); }
		IDXGIAdapter1* getAdapter() const { return m_device.getAdapter(); }
		ID3D12CommandQueue* getCommandQueue() const { return m_commandQueue.getCommandQueue(); }
		IDXGISwapChain3* getSwapChain() const { return m_swapChain.getSwapChain(); }
		ID3D12DescriptorHeap* getRTVHeap() const { return m_rtv.getRTVHeap(); }
		ID3D12Resource* getRTV(UINT index) const { return m_rtv.getRenderTarget(index); }
		ID3D12GraphicsCommandList* getCommandList() const { return m_commandList.getCommandList(); }
		ID3D12CommandAllocator* getCommandAllocator() const { return m_commandList.getCommandAllocator(); }
		ID3D12Fence* getFence() const { return m_fence.getFence(); }

	private:

		uint32_t m_width = 0, m_height = 0;
		int m_vsyncInterval = 0;

		Vec4 m_clearColor = Vec4::zero();

		D12Device m_device;
		D12CommandQueue m_commandQueue;
		D12SwapChain m_swapChain;
		D12RenderTarget m_rtv;
		D12CommandList m_commandList;
		D12Fence m_fence;
		D12srvHeap m_srvHeap;

	};

}
