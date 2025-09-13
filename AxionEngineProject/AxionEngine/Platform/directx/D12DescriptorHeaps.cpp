#include "axpch.h"
#include "D12DescriptorHeaps.h"

namespace Axion {

	////////////////////////////////////////////////////////////////////////////
	///////////// RTV Heap /////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////

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

		// ----- Create RTV heap -----
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
		m_nextIndex.store(0);
		std::lock_guard<std::mutex> lock(m_freeListMutex);
		std::queue<uint32_t>().swap(m_freeList);
	}

	uint32_t D12rtvHeap::allocate() {
		std::lock_guard<std::mutex> lock(m_freeListMutex);
		if (!m_freeList.empty()) {
			uint32_t index = m_freeList.front();
			m_freeList.pop();
			return index;
		}

		uint32_t idx = m_nextIndex.fetch_add(1, std::memory_order_relaxed);
		AX_CORE_ASSERT(idx < m_numDescriptors, "Out of RTV heap descriptors");
		return idx;
	}

	void D12rtvHeap::free(uint32_t index) {
		AX_CORE_ASSERT(index < m_numDescriptors, "Trying to free invalid RTV descriptor index");
		std::lock_guard<std::mutex> lock(m_freeListMutex);
		m_freeList.push(index);
	}

	D3D12_CPU_DESCRIPTOR_HANDLE D12rtvHeap::getCpuHandle(uint32_t index) const {
		CD3DX12_CPU_DESCRIPTOR_HANDLE handle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart(), index, m_descriptorSize);
		return handle;
	}



	////////////////////////////////////////////////////////////////////////////
	///////////// SRV Heap /////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////

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

		// ----- Create SRV heap -----
		HRESULT hr = device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&m_srvHeap));
		AX_THROW_IF_FAILED_HR(hr, "Failed to create srv descriptor heap");

		m_descriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

		AX_CORE_LOG_TRACE("Successfully created SRV heap with {0} descriptors", numDescriptors);
	}

	void D12srvHeap::release() {
		m_srvHeap.Reset();
		m_nextIndex.store(0);
		std::lock_guard<std::mutex> lock(m_freeListMutex);
		std::queue<uint32_t>().swap(m_freeList);
	}

	uint32_t D12srvHeap::allocate() {
		std::lock_guard<std::mutex> lock(m_freeListMutex);
		if (!m_freeList.empty()) {
			uint32_t index = m_freeList.front();
			m_freeList.pop();
			return index;
		}

		uint32_t idx = m_nextIndex.fetch_add(1, std::memory_order_relaxed);
		AX_CORE_ASSERT(idx < m_numDescriptors, "Out of srv heap descriptors");
		return idx;
	}

	void D12srvHeap::free(uint32_t index) {
		AX_CORE_ASSERT(index < m_numDescriptors, "Trying to free invalid SRV descriptor index");
		std::lock_guard<std::mutex> lock(m_freeListMutex);
		m_freeList.push(index);
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



	////////////////////////////////////////////////////////////////////////////
	///////////// DSV Heap /////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////

	D12dsvHeap::D12dsvHeap() {}

	D12dsvHeap::~D12dsvHeap() {
		release();
	}

	void D12dsvHeap::initialize(ID3D12Device* device, uint32_t numDescriptors) {
		m_numDescriptors = numDescriptors;

		D3D12_DESCRIPTOR_HEAP_DESC desc = {};
		desc.NumDescriptors = numDescriptors;
		desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
		desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

		// ----- Create DSV heap -----
		HRESULT hr = device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&m_dsvHeap));
		AX_THROW_IF_FAILED_HR(hr, "Failed to create dsv descriptor heap");

		m_descriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);

		AX_CORE_LOG_TRACE("Successfully created DSV heap with {0} descriptors", numDescriptors);
	}

	void D12dsvHeap::release() {
		m_dsvHeap.Reset();
		m_nextIndex.store(0);
		std::lock_guard<std::mutex> lock(m_freeListMutex);
		std::queue<uint32_t>().swap(m_freeList);
	}

	uint32_t D12dsvHeap::allocate() {
		std::lock_guard<std::mutex> lock(m_freeListMutex);
		if (!m_freeList.empty()) {
			uint32_t index = m_freeList.front();
			m_freeList.pop();
			return index;
		}

		uint32_t idx = m_nextIndex.fetch_add(1, std::memory_order_relaxed);
		AX_CORE_ASSERT(idx < m_numDescriptors, "Out of dsv heap descriptors");
		return idx;
	}

	void D12dsvHeap::free(uint32_t index) {
		AX_CORE_ASSERT(index < m_numDescriptors, "Trying to free invalid DSV descriptor index");
		std::lock_guard<std::mutex> lock(m_freeListMutex);
		m_freeList.push(index);
	}

	D3D12_CPU_DESCRIPTOR_HANDLE D12dsvHeap::getCpuHandle(uint32_t index) const {
		D3D12_CPU_DESCRIPTOR_HANDLE handle = m_dsvHeap->GetCPUDescriptorHandleForHeapStart();
		handle.ptr += index * m_descriptorSize;
		return handle;
	}

}
