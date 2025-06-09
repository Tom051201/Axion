#pragma once

#include "axpch.h"

#include "Axion/render/GraphicsContext.h"

#include "D12Device.h"
#include "D12CommandQueue.h"
#include "D12SwapChain.h"
#include "D12RenderTarget.h"
#include "D12CommandList.h"
#include "D12Fence.h"

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

		inline void* getNativeContext() const override { return (void*)this; }

		inline ID3D12Device* getDevice() const { return m_device.getDevice(); }
		inline ID3D12GraphicsCommandList* getCommandList() const { return m_commandList.getCommandList(); }
		inline const D12RenderTarget* getRenderTarget() const { return &m_rtv; }
		inline ID3D12CommandQueue* getCommandQueue() const { return m_commandQueue.getCommandQueue(); }

	private:

		void waitForPreviousFrame();

		uint32_t m_width = 0, m_height = 0;

		D12Device m_device;
		D12CommandQueue m_commandQueue;
		D12SwapChain m_swapChain;
		D12RenderTarget m_rtv;
		D12CommandList m_commandList;
		D12Fence m_fence;

	};

}
