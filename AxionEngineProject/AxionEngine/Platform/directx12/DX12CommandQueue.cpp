#include "axpch.h"
#include "DX12CommandQueue.h"

#include "AxionEngine/Source/core/Core.h"

namespace Axion {

	DX12CommandQueue::~DX12CommandQueue() {
		release();
	}

	void DX12CommandQueue::initialize(ID3D12Device* device) {
		D3D12_COMMAND_QUEUE_DESC desc = {};
		desc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
		AX_THROW_IF_FAILED_HR(device->CreateCommandQueue(&desc, IID_PPV_ARGS(&m_cmdQueue)), "Failed to create command queue");
		AX_CORE_LOG_TRACE("Successfully created command queue");

		#ifdef AX_DEBUG
		m_cmdQueue->SetName(L"CommandQueue");
		#endif
	}

	void DX12CommandQueue::release() {
		m_cmdQueue.Reset();
	}

	void DX12CommandQueue::executeCommandList(ID3D12CommandList* cmdList) {
		AX_CORE_ASSERT(cmdList != nullptr, "Null command list");
		ID3D12CommandList* lists[] = { cmdList };
		m_cmdQueue->ExecuteCommandLists(1, lists);
	}

	void DX12CommandQueue::executeCommandLists(const std::vector<ID3D12CommandList*>& cmdLists) {
		m_cmdQueue->ExecuteCommandLists(static_cast<UINT>(cmdLists.size()), cmdLists.data());
	}

}
