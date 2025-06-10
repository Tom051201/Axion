#pragma once

#include "axpch.h"

#include "Axion/render/GraphicsContext.h"

#include "platform/directx/D12Device.h"
#include "platform/directx/D12CommandQueue.h"
#include "platform/directx/D12SwapChain.h"
#include "platform/directx/D12RenderTarget.h"
#include "platform/directx/D12CommandList.h"
#include "platform/directx/D12Fence.h"

namespace Axion {

	class D12Context : public GraphicsContext {
	public:

		~D12Context() override;

		void initialize(void* hwnd, uint32_t width, uint32_t height) override;
		void shutdown() override;

		void present();
		void clear(float r, float g, float b, float a);
		void beginFrame();
		void endFrame();

		void resize(uint32_t width, uint32_t height);

		void activateVsync() { m_vsyncInterval = 1; };
		void deactivateVsync() { m_vsyncInterval = 0; };

		inline void* getNativeContext() const override { return (void*)this; }

		inline ID3D12Device* getDevice() const { return m_device.getDevice(); }
		inline IDXGIFactory6* getFactory() const { return m_device.getFactory(); }
		inline IDXGIAdapter1* getAdapter() const { return m_device.getAdapter(); }
		inline ID3D12CommandQueue* getCommandQueue() const { return m_commandQueue.getCommandQueue(); }
		inline IDXGISwapChain3* getSwapChain() const { return m_swapChain.getSwapChain(); }
		inline ID3D12DescriptorHeap* getRTVHeap() const { return m_rtv.getRTVHeap(); }
		inline ID3D12Resource* getRTV(UINT index) const { return m_rtv.getRenderTarget(index); }
		inline ID3D12GraphicsCommandList* getCommandList() const { return m_commandList.getCommandList(); }
		inline ID3D12CommandAllocator* getCommandAllocator() const { return m_commandList.getCommandAllocator(); }
		inline ID3D12Fence* getFence() const { return m_fence.getFence(); }

	private:

		void waitForPreviousFrame();

		uint32_t m_width = 0, m_height = 0;
		int m_vsyncInterval = 0;

		D12Device m_device;
		D12CommandQueue m_commandQueue;
		D12SwapChain m_swapChain;
		D12RenderTarget m_rtv;
		D12CommandList m_commandList;
		D12Fence m_fence;

	};

}
