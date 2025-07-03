#pragma once
#include "axpch.h"

#include "AxionEngine/Source/core/Math.h"
#include "AxionEngine/Source/render/GraphicsContext.h"

#include "AxionEngine/Platform/directx/D12Device.h"
#include "AxionEngine/Platform/directx/D12CommandQueue.h"
#include "AxionEngine/Platform/directx/D12SwapChain.h"
#include "AxionEngine/Platform/directx/D12CommandList.h"
#include "AxionEngine/Platform/directx/D12Fence.h"
#include "AxionEngine/Platform/directx/D12srvHeap.h"
#include "AxionEngine/Platform/directx/D12rtvHeap.h"

namespace Axion {

	class D12Context : public GraphicsContext {
	public:

		~D12Context() override;

		void initialize(void* hwnd, uint32_t width, uint32_t height) override;
		void shutdown() override;

		void prepareRendering() override;
		void finishRendering() override;

		inline void* getNativeContext() const override { return (void*)this; }
		
		void setClearColor(const Vec4& color);
		void clear();

		void resize(uint32_t width, uint32_t height);

		void activateVsync() { m_vsyncInterval = 1; };
		void deactivateVsync() { m_vsyncInterval = 0; };
		
		std::string getGpuName() const override;
		std::string getGpuDriverVersion() const override;
		uint64_t getVramMB() const override;

		void waitForPreviousFrame();

		D12Device& getDeviceWrapper() { return m_device; }
		D12CommandQueue& getCommandQueueWrapper() { return m_commandQueue; }
		D12rtvHeap& getRtvHeapWrapper() { return m_rtvHeap; }
		D12SwapChain& getSwapChainWrapper() { return m_swapChain; }
		D12CommandList& getCommandListWrapper() { return m_commandList; }
		D12Fence& getFenceWrapper() { return m_fence; }
		D12srvHeap& getSrvHeapWrapper() { return m_srvHeap; }

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
		D12srvHeap m_srvHeap;

	};

}
