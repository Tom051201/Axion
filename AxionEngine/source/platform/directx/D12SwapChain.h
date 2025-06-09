#pragma once

#include "AxionSettings.h"

namespace Axion {

	class D12SwapChain {
	public:

		D12SwapChain() = default;
		~D12SwapChain();

		void initialize(HWND hwnd, IDXGIFactory6* factory, ID3D12CommandQueue* cmdQueue, UINT width, UINT height);
		void release();

		void resize(UINT width, UINT height, ID3D12Device* device, D3D12_CPU_DESCRIPTOR_HANDLE rtvHeapStart, UINT rtvDescriptorSize,
			std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>>& renderTargets);

		inline IDXGISwapChain3* getSwapChain() const { return m_swapChain.Get(); }
		inline UINT getFrameIndex() const { return m_frameIndex; }
		inline void setFrameIndex(UINT frameIndex) { m_frameIndex = frameIndex; }

	private:

		const UINT m_frameCount = AX_MAX_SWAPCHAIN_BUFFERS;
		UINT m_frameIndex = 0;
		Microsoft::WRL::ComPtr<IDXGISwapChain3> m_swapChain;
		Microsoft::WRL::ComPtr<IDXGISwapChain1> m_tempsc1;

		HWND m_hwnd = nullptr;
	};

}
