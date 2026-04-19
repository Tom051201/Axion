#pragma once

#include "AxionEngine/Source/EngineConfig.h"
#include "AxionEngine/Source/render/SwapChainSpecification.h"

namespace Axion {

	class DX12Context;

	class DX12SwapChain {
	public:

		DX12SwapChain() = default;
		~DX12SwapChain();

		void initialize(HWND hwnd, IDXGIFactory6* factory, ID3D12CommandQueue* cmdQueue, const SwapChainSpecification& spec);
		void release();

		void resize(UINT width, UINT height);
		void present(UINT syncInterval = 1, UINT flags = 0);
		void advanceFrame();
		void setAsRenderTarget();
		void clear(const float clearColor[4]);

		IDXGISwapChain3* getSwapChain() const { return m_swapChain.Get(); }
		UINT getFrameIndex() const { return m_frameIndex; }
		void setFrameIndex(UINT frameIndex) { m_frameIndex = frameIndex; }

		ID3D12Resource* getBackBuffer(uint32_t index) const { AX_CORE_ASSERT(index < Config::MaxSwapchainBuffers, "Invalid back buffer index"); return m_backBuffers[index].Get(); }
		ID3D12Resource* getDepthBuffer(uint32_t index) const { AX_CORE_ASSERT(index < Config::MaxSwapchainBuffers, "Invalid depth buffer index"); return m_depthBuffers[index].Get(); }

		D3D12_CPU_DESCRIPTOR_HANDLE getBackBufferRtv(uint32_t index) const;
		D3D12_CPU_DESCRIPTOR_HANDLE getDepthBufferDsv(uint32_t index) const;

		uint32_t getRtvHeapIndex(uint32_t framecount) const { return m_rtvHeapIndices[framecount]; }
		uint32_t getDsvHeapIndex(uint32_t framecount) const { return m_rtvHeapIndices[framecount]; }
	
	private:

		DX12Context* m_context = nullptr;
		Microsoft::WRL::ComPtr<IDXGISwapChain3> m_swapChain;
		SwapChainSpecification m_specification;

		UINT m_frameIndex = 0;

		Microsoft::WRL::ComPtr<ID3D12Resource> m_backBuffers[Config::MaxSwapchainBuffers];
		Microsoft::WRL::ComPtr<ID3D12Resource> m_depthBuffers[Config::MaxSwapchainBuffers];

		uint32_t m_dsvHeapIndices[Config::MaxSwapchainBuffers];
		uint32_t m_rtvHeapIndices[Config::MaxSwapchainBuffers];

	};

}
