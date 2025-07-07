#pragma once

namespace Axion {

	class D12dsvHeap {
	public:

		D12dsvHeap();
		~D12dsvHeap();

		void initialize(ID3D12Device* device, uint32_t numDescriptors);
		void release();

		uint32_t allocate();

		ID3D12DescriptorHeap* getHeap() const { return m_dsvHeap.Get(); }
		D3D12_CPU_DESCRIPTOR_HANDLE getCpuHandle(uint32_t index) const;

	private:

		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_dsvHeap;

		uint32_t m_descriptorSize = 0;
		std::atomic<uint32_t> m_nextIndex = 0;
		uint32_t m_numDescriptors = 0;

	};

}
