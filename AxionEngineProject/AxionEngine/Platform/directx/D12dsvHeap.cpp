#include "axpch.h"
#include "D12dsvHeap.h"

namespace Axion {
	
	D12dsvHeap::D12dsvHeap(){}
	
	D12dsvHeap::~D12dsvHeap() {
		release();
	}

	void D12dsvHeap::initialize(ID3D12Device* device, uint32_t numDescriptors) {
		m_numDescriptors = numDescriptors;

		D3D12_DESCRIPTOR_HEAP_DESC desc = {};
		desc.NumDescriptors = numDescriptors;
		desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
		desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

		AX_THROW_IF_FAILED_HR(device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&m_dsvHeap)), "Failed to create dsv descriptor heap");

		m_descriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);

		AX_CORE_LOG_TRACE("Successfully created DSV heap with {0} descriptors", numDescriptors);
	}

	void D12dsvHeap::release() {
		m_dsvHeap.Reset();
	}

	uint32_t D12dsvHeap::allocate() {
		uint32_t idx = m_nextIndex.fetch_add(1, std::memory_order_relaxed);
		AX_ASSERT(idx < m_numDescriptors, "Out of dsv heap descriptors");
		return idx;
	}

	D3D12_CPU_DESCRIPTOR_HANDLE D12dsvHeap::getCpuHandle(uint32_t index) const {
		D3D12_CPU_DESCRIPTOR_HANDLE handle = m_dsvHeap->GetCPUDescriptorHandleForHeapStart();
		handle.ptr += index * m_descriptorSize;
		return handle;
	}
}
