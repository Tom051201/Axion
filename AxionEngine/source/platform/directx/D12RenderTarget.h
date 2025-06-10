#pragma once

#include "AxionSettings.h"

namespace Axion {

	class D12RenderTarget {
	public:

		D12RenderTarget() = default;
		~D12RenderTarget();

		void initialize(ID3D12Device* device, IDXGISwapChain3* swapChain);
		void release();

		void resetRTVs();
		void resize(ID3D12Device* device, IDXGISwapChain3* swapChain);

		inline ID3D12DescriptorHeap* getRTVHeap() const { return m_rtvHeap.Get(); }
		inline ID3D12Resource* getRenderTarget(UINT index) const { return m_renderTargets[index].Get(); }
		inline UINT getRTVDescriptorSize() const { return m_rtvDescriptorSize; }

	private:
	
		const UINT m_frameCount = AX_MAX_SWAPCHAIN_BUFFERS;
		UINT m_rtvDescriptorSize = 0;
		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_rtvHeap;
		Microsoft::WRL::ComPtr<ID3D12Resource> m_renderTargets[AX_MAX_SWAPCHAIN_BUFFERS];
	
	};

}
