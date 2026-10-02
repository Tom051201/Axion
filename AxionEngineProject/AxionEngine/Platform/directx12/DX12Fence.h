#pragma once

#include <windows.h>
#include <d3d12.h>
#include <wrl/client.h>

namespace Axion {

	class DX12Fence {
	public:

		DX12Fence() = default;
		~DX12Fence();

		void initialize(ID3D12Device* device);
		void release();

		void signal(ID3D12CommandQueue* queue, UINT64 fenceValue);
		void wait(UINT64 fenceValue);
		bool hasCompleted(UINT64 fenceValue) const;

		ID3D12Fence* getFence() const { return m_fence.Get(); }
		HANDLE getFenceEvent() const { return m_fenceEvent; }

	private:

		Microsoft::WRL::ComPtr<ID3D12Fence> m_fence;
		HANDLE m_fenceEvent;
	
	};
}
