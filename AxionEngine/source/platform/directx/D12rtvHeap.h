#pragma once
#include "axpch.h"

namespace Axion {

	class D12rtvHeap {
	public:

		D12rtvHeap();
		~D12rtvHeap();

		void initialize(ID3D12Device* device, uint32_t numDescriptors);
		void release();

		uint32_t allocate();

		ID3D12DescriptorHeap* getHeap() const { return m_rtvHeap.Get(); }
		D3D12_CPU_DESCRIPTOR_HANDLE getCpuHandle(uint32_t index) const;
		uint32_t getDescriptorSize() const { return m_descriptorSize; }

	private:

		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_rtvHeap;

		uint32_t m_descriptorSize = 0;
		std::atomic<uint32_t> m_nextIndex = 0;
		uint32_t m_numDescriptors = 0;

	};

}
