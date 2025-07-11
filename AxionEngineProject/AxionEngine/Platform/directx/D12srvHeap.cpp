#include "axpch.h"
#include "D12srvHeap.h"

namespace Axion {

	D12srvHeap::D12srvHeap() {}

	D12srvHeap::~D12srvHeap() {
		release();
	}

	void D12srvHeap::initialize(ID3D12Device* device, uint32_t numDescriptors) {
		m_numDescriptors = numDescriptors;

		D3D12_DESCRIPTOR_HEAP_DESC desc = {};
		desc.NumDescriptors = numDescriptors;
		desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

		AX_THROW_IF_FAILED_HR(device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&m_srvHeap)), "Failed to create srv descriptor heap");

		m_descriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

		AX_CORE_LOG_TRACE("Successfully created SRV heap with {0} descriptors", numDescriptors);
	}

	void D12srvHeap::release() {
		m_srvHeap.Reset();
	}

	uint32_t D12srvHeap::allocate() {
		uint32_t idx = m_nextIndex.fetch_add(1, std::memory_order_relaxed);
		AX_CORE_ASSERT(idx < m_numDescriptors, "Out of srv heap descriptors");
		return idx;
	}

	D3D12_CPU_DESCRIPTOR_HANDLE D12srvHeap::getCpuHandle(uint32_t index) const {
		D3D12_CPU_DESCRIPTOR_HANDLE handle = m_srvHeap->GetCPUDescriptorHandleForHeapStart();
		handle.ptr += index * m_descriptorSize;
		return handle;
	}

	D3D12_GPU_DESCRIPTOR_HANDLE D12srvHeap::getGpuHandle(uint32_t index) const {
		D3D12_GPU_DESCRIPTOR_HANDLE handle = m_srvHeap->GetGPUDescriptorHandleForHeapStart();
		handle.ptr += index * m_descriptorSize;
		return handle;
	}

}
