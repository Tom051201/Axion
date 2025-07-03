#pragma once
#include "axpch.h"

namespace Axion {

	class D12srvHeap {
	public:

		D12srvHeap();
		~D12srvHeap();

		void initialize(ID3D12Device* device, uint32_t numDescriptors);
		void release();

		uint32_t allocate();
		
		ID3D12DescriptorHeap* getHeap() const { return m_srvHeap.Get(); }
		D3D12_CPU_DESCRIPTOR_HANDLE getCpuHandle(uint32_t index) const;
		D3D12_GPU_DESCRIPTOR_HANDLE getGpuHandle(uint32_t index) const;

	private:

		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_srvHeap;

		uint32_t m_descriptorSize = 0;
		std::atomic<uint32_t> m_nextIndex = 0;
		uint32_t m_numDescriptors = 0;

	};

}
