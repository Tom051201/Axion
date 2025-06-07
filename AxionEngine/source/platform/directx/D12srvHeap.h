#pragma once

#include "axpch.h"

namespace Axion {

	class D12srvHeap {
	public:

		void initialize(ID3D12Device* device, UINT maxDescriptors = 1024);

		D3D12_CPU_DESCRIPTOR_HANDLE allocateO();
		UINT allocate();
		D3D12_CPU_DESCRIPTOR_HANDLE getCPUHandle(UINT index) const;
		D3D12_GPU_DESCRIPTOR_HANDLE getGPUHandle(UINT index) const;

		ID3D12DescriptorHeap* getHeap() const { return m_srvHeap.Get(); }

	private:

		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_srvHeap;
		UINT m_descriptorSize = 0;
		UINT m_maxDescriptors = 0;
		UINT m_currentIndex = 0;

	};

}
