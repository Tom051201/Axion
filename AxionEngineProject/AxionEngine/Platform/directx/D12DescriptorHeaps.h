#pragma once
#include "axpch.h"

namespace Axion {

	////////////////////////////////////////////////////////////////////////////
	///////////// RTV Heap /////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////

	class D12rtvHeap {
	public:

		D12rtvHeap();
		~D12rtvHeap();

		void initialize(ID3D12Device* device, uint32_t numDescriptors);
		void release();

		uint32_t allocate();
		void free(uint32_t index);

		ID3D12DescriptorHeap* getHeap() const { return m_rtvHeap.Get(); }
		D3D12_CPU_DESCRIPTOR_HANDLE getCpuHandle(uint32_t index) const;
		uint32_t getDescriptorSize() const { return m_descriptorSize; }

	private:

		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_rtvHeap;

		uint32_t m_descriptorSize = 0;
		uint32_t m_numDescriptors = 0;

		std::atomic<uint32_t> m_nextIndex = 0;
		std::queue<uint32_t> m_freeList;
		std::mutex m_freeListMutex;

	};



	////////////////////////////////////////////////////////////////////////////
	///////////// SRV Heap /////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////

	class D12srvHeap {
	public:

		D12srvHeap();
		~D12srvHeap();

		void initialize(ID3D12Device* device, uint32_t numDescriptors, bool isShaderVisible, uint32_t frameCount = 3);
		void release();

		uint32_t allocate();
		uint32_t allocateRange(uint32_t count);
		uint32_t allocateStatic();
		void free(uint32_t index);

		void reserve(uint32_t numDescriptors);
		void nextFrame();

		ID3D12DescriptorHeap* getHeap() const { return m_srvHeap.Get(); }
		D3D12_CPU_DESCRIPTOR_HANDLE getCpuHandle(uint32_t index) const;
		D3D12_GPU_DESCRIPTOR_HANDLE getGpuHandle(uint32_t index) const;

	private:

		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_srvHeap;

		uint32_t m_descriptorSize = 0;
		uint32_t m_numDescriptors = 0;

		uint32_t m_frameCount = 3;
		uint32_t m_currentFrame = 0;
		std::vector<uint32_t> m_frameNextIndex;

		std::atomic<uint32_t> m_nextIndex = 0;
		std::queue<uint32_t> m_freeList;
		std::mutex m_freeListMutex;

		uint32_t m_reservedCount = 0;
		uint32_t m_staticIndex = 0;

	};



	////////////////////////////////////////////////////////////////////////////
	///////////// DSV Heap /////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////

	class D12dsvHeap {
	public:

		D12dsvHeap();
		~D12dsvHeap();

		void initialize(ID3D12Device* device, uint32_t numDescriptors);
		void release();

		uint32_t allocate();
		void free(uint32_t index);

		ID3D12DescriptorHeap* getHeap() const { return m_dsvHeap.Get(); }
		D3D12_CPU_DESCRIPTOR_HANDLE getCpuHandle(uint32_t index) const;

	private:

		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_dsvHeap;

		uint32_t m_descriptorSize = 0;
		uint32_t m_numDescriptors = 0;

		std::atomic<uint32_t> m_nextIndex = 0;
		std::queue<uint32_t> m_freeList;
		std::mutex m_freeListMutex;

	};

}
