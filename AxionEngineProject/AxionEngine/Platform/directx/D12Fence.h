#pragma once

namespace Axion {

	class D12Fence {
	public:

		D12Fence() = default;
		~D12Fence();

		void initialize(ID3D12Device* device);
		void release();

		void waitForGPU();
		void signalAndWait();
		bool hasCompleted() const;

		ID3D12Fence* getFence() const { return m_fence.Get(); }
		UINT64 getFenceValue() const { return m_fenceValue; }
		HANDLE getFenceEvent() const { return m_fenceEvent; }
		void incrFenceValue() { m_fenceValue++; }

	private:

		Microsoft::WRL::ComPtr<ID3D12Fence> m_fence;
		UINT64 m_fenceValue;
		HANDLE m_fenceEvent;
	
	};
}
