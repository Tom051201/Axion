#include "axpch.h"
#include "D12srvHeap.h"

namespace Axion {

	void D12srvHeap::initialize(ID3D12Device* device, UINT maxDescriptors) {
		AX_CORE_LOG_WARN("Initializing SRV heap...");

		m_maxDescriptors = maxDescriptors;

		D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
		heapDesc.NumDescriptors = m_maxDescriptors;
		heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
		heapDesc.NodeMask = 0;

		HRESULT hr = device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&m_srvHeap));
		AX_CORE_ASSERT(SUCCEEDED(hr), "Failed to create SRV descriptor heap");

		m_descriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		m_currentIndex = 0;

	}

	D3D12_CPU_DESCRIPTOR_HANDLE D12srvHeap::allocateO() {
		AX_CORE_LOG_WARN("Allocating SRV descriptor...");

		AX_CORE_ASSERT(m_currentIndex < m_maxDescriptors, "SRV Heap overflow!");

		D3D12_CPU_DESCRIPTOR_HANDLE handle = m_srvHeap->GetCPUDescriptorHandleForHeapStart();
		handle.ptr += static_cast<SIZE_T>(m_currentIndex) * m_descriptorSize;
		m_currentIndex++;

		return handle;
	}

	D3D12_CPU_DESCRIPTOR_HANDLE D12srvHeap::getCPUHandle(UINT index) const {
		D3D12_CPU_DESCRIPTOR_HANDLE handle = m_srvHeap->GetCPUDescriptorHandleForHeapStart();
		handle.ptr += static_cast<SIZE_T>(index) * m_descriptorSize;
		return handle;
	}

	D3D12_GPU_DESCRIPTOR_HANDLE D12srvHeap::getGPUHandle(UINT index) const {
		D3D12_GPU_DESCRIPTOR_HANDLE handle = m_srvHeap->GetGPUDescriptorHandleForHeapStart();
		handle.ptr += static_cast<SIZE_T>(index) * m_descriptorSize;
		return handle;
	}

	UINT D12srvHeap::allocate() {
		AX_CORE_ASSERT(m_currentIndex < m_maxDescriptors, "SRV Heap overflow!");
		return m_currentIndex++;
	}

}
