#include "axpch.h"
#include "D12rtvHeap.h"

namespace Axion {

	D12rtvHeap::D12rtvHeap() {}

	D12rtvHeap::~D12rtvHeap() {
		release();
	}

	void D12rtvHeap::initialize(ID3D12Device* device, uint32_t numDescriptors) {
		if (m_rtvHeap) release();

		m_numDescriptors = numDescriptors;

		D3D12_DESCRIPTOR_HEAP_DESC desc = {};
		desc.NumDescriptors = numDescriptors;
		desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
		desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

		HRESULT hr = device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&m_rtvHeap));
		AX_THROW_IF_FAILED_HR(hr, "Failed to create RTV descriptor heap");

		m_descriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

		#ifdef AX_DEBUG
		m_rtvHeap->SetName(L"RTV Heap");
		#endif

		AX_CORE_LOG_TRACE("Successfully created RTV heap with {0} descriptors", numDescriptors);

	}

	void D12rtvHeap::release() {
		m_rtvHeap.Reset();
	}

	uint32_t D12rtvHeap::allocate() {
		uint32_t idx = m_nextIndex.fetch_add(1, std::memory_order_relaxed);
		AX_ASSERT(idx < m_numDescriptors, "Out of RTV heap descriptors");
		return idx;
	}

	D3D12_CPU_DESCRIPTOR_HANDLE D12rtvHeap::getCpuHandle(uint32_t index) const {
		CD3DX12_CPU_DESCRIPTOR_HANDLE handle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart(), index, m_descriptorSize);
		return handle;
	}

}
