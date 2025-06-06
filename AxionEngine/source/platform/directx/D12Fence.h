#pragma once

namespace Axion {

	class D12Fence {
	public:

		D12Fence() = default;
		~D12Fence();

		void initialize(ID3D12Device* device);
		void release();

		inline ID3D12Fence* getFence() const { return m_fence.Get(); }
		inline UINT64 getFenceValue() const { return m_fenceValue; }
		inline HANDLE getFenceEvent() const { return m_fenceEvent; }
		inline void incrFenceValue() { m_fenceValue++; }

	private:

		Microsoft::WRL::ComPtr<ID3D12Fence> m_fence;
		UINT64 m_fenceValue;
		HANDLE m_fenceEvent;
	
	};
}
