#pragma once

#include "AxionEngine/Source/AxionSettings.h"

namespace Axion {

	class D12Context;


	class D12SwapChain {
	public:

		D12SwapChain() = default;
		~D12SwapChain();

		void initialize(HWND hwnd, IDXGIFactory6* factory, ID3D12CommandQueue* cmdQueue, UINT width, UINT height);
		void release();

		void resize(UINT width, UINT height);
		void present(UINT syncInterval = 1, UINT flags = 0);
		void advanceFrame();
		void setAsRenderTarget();

		inline IDXGISwapChain3* getSwapChain() const { return m_swapChain.Get(); }

		inline UINT getFrameIndex() const { return m_frameIndex; }
		inline void setFrameIndex(UINT frameIndex) { m_frameIndex = frameIndex; }

		ID3D12Resource* getBackBuffer(uint32_t index) const { AX_ASSERT(index < m_frameCount, "Invalid back buffer index"); return m_backBuffers[index].Get(); }
		D3D12_CPU_DESCRIPTOR_HANDLE getBackBufferRtv(uint32_t index) const;
		uint32_t getRtvHeapIndex(uint32_t framecount) const { return m_rtvHeapIndices[framecount]; }

	private:

		D12Context* m_context = nullptr;

		Microsoft::WRL::ComPtr<IDXGISwapChain3> m_swapChain;
		const UINT m_frameCount = AX_MAX_SWAPCHAIN_BUFFERS;
		UINT m_frameIndex = 0;

		Microsoft::WRL::ComPtr<ID3D12Resource> m_backBuffers[AX_MAX_SWAPCHAIN_BUFFERS];
		uint32_t m_rtvHeapIndices[AX_MAX_SWAPCHAIN_BUFFERS];
	};

}
