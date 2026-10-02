#include "axpch.h"
#include "DX12CommandManager.h"

#include "AxionEngine/Platform/directx12/DX12RenderContext.h"
#include "AxionEngine/Platform/directx12/DX12Context.h"

namespace Axion {

	DX12CommandManager::~DX12CommandManager() {
		release();
	}

	void DX12CommandManager::initialize(ID3D12Device* device, uint32_t numFrames, uint32_t maxThreads) {
		m_device = device;
		m_allocators.resize(numFrames);
		m_contextPool.resize(numFrames);
		m_availableContexts.resize(numFrames);
		m_activeContexts.resize(numFrames);

		uint32_t preallocCount = 16;
		for (uint32_t f = 0; f < numFrames; f++) {
			for (uint32_t t = 0; t < preallocCount; t++) {
				Microsoft::WRL::ComPtr<ID3D12CommandAllocator> alloc;
				AX_THROW_IF_FAILED_HR(device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&alloc)), "Failed to create allocator");
				m_allocators[f].push_back(alloc);
			}
		}
	}

	void DX12CommandManager::release() {
		m_contextPool.clear();
		m_availableContexts.clear();
		m_activeContexts.clear();
		m_allocators.clear();
	}

	void DX12CommandManager::beginFrame(uint32_t frameIndex) {
		m_currentFrameIndex = frameIndex;
		m_contextCounter = 0;

		m_availableContexts[m_currentFrameIndex].clear();
		for (auto& ctx : m_contextPool[m_currentFrameIndex]) {
			m_availableContexts[m_currentFrameIndex].push_back(ctx.get());
		}

		m_activeContexts[m_currentFrameIndex].clear();
	}

	DX12RenderContext* DX12CommandManager::acquireContext(uint32_t threadIndex) {
		std::lock_guard<std::mutex> lock(m_poolMutex);

		// -- Unique ID for this specific job execution //
		uint32_t contextId = m_contextCounter++;

		if (contextId >= m_allocators[m_currentFrameIndex].size()) {
			Microsoft::WRL::ComPtr<ID3D12CommandAllocator> newAlloc;
			AX_THROW_IF_FAILED_HR(m_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&newAlloc)), "Failed to create dynamic allocator");
			m_allocators[m_currentFrameIndex].push_back(newAlloc);
		}

		auto allocator = m_allocators[m_currentFrameIndex][contextId].Get();
		AX_THROW_IF_FAILED_HR(allocator->Reset(), "Failed to reset command allocator");

		DX12RenderContext* context = nullptr;

		if (m_availableContexts[m_currentFrameIndex].empty()) {
			Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> cmdList;
			AX_THROW_IF_FAILED_HR(m_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, allocator, nullptr, IID_PPV_ARGS(&cmdList)), "Failed to create command list");
			cmdList->Close();

			m_contextPool[m_currentFrameIndex].push_back(MakeScope<DX12RenderContext>(cmdList));
			context = m_contextPool[m_currentFrameIndex].back().get();
		}
		else {
			context = m_availableContexts[m_currentFrameIndex].back();
			m_availableContexts[m_currentFrameIndex].pop_back();
		}

		AX_THROW_IF_FAILED_HR(context->getCmdList()->Reset(allocator, nullptr), "Failed to reset command list");

		auto* dx12Context = static_cast<DX12Context*>(GraphicsContext::get()->getNativeContext());
		ID3D12DescriptorHeap* descriptorHeaps[] = { dx12Context->getSrvHeapWrapper().getHeap() };
		context->getCmdList()->SetDescriptorHeaps(1, descriptorHeaps);

		context->begin();

		if (threadIndex != 0) {
			m_activeContexts[m_currentFrameIndex].push_back(context);
		}

		return context;
	}

	std::vector<ID3D12CommandList*> DX12CommandManager::getActiveCommandLists() {
		std::lock_guard<std::mutex> lock(m_poolMutex);
		std::vector<ID3D12CommandList*> activeLists;
		activeLists.reserve(m_activeContexts[m_currentFrameIndex].size());

		for (auto* context : m_activeContexts[m_currentFrameIndex]) {
			activeLists.push_back(context->getCmdList());
		}
		return activeLists;
	}

}
