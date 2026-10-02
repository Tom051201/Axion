#pragma once

#include <vector>
#include <mutex>
#include <d3d12.h>
#include <vector>
#include <wrl/client.h>

#include "AxionEngine/Source/core/Core.h"
#include "AxionEngine/Platform/directx12/DX12RenderContext.h"

namespace Axion {

	class DX12CommandManager {
	public:

		DX12CommandManager() = default;
		~DX12CommandManager();

		void initialize(ID3D12Device* device, uint32_t numFrames, uint32_t maxThreads);
		void release();

		void beginFrame(uint32_t frameIndex);

		DX12RenderContext* acquireContext(uint32_t threadIndex);
		std::vector<ID3D12CommandList*> getActiveCommandLists();

	private:

		ID3D12Device* m_device = nullptr;
		uint32_t m_currentFrameIndex = 0;
		uint32_t m_contextCounter = 0;

		std::vector<std::vector<Microsoft::WRL::ComPtr<ID3D12CommandAllocator>>> m_allocators;
		std::vector<std::vector<Scope<DX12RenderContext>>> m_contextPool;
		std::vector<std::vector<DX12RenderContext*>> m_availableContexts;
		std::vector<std::vector<DX12RenderContext*>> m_activeContexts;

		std::mutex m_poolMutex;

	};

}
