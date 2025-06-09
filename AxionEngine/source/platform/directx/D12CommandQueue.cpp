#include "axpch.h"
#include "D12CommandQueue.h"

#include "Axion/Core.h"

namespace Axion {

	D12CommandQueue::~D12CommandQueue() {
		release();
	}

	void D12CommandQueue::initialize(ID3D12Device* device) {
		D3D12_COMMAND_QUEUE_DESC desc = {};
		desc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
		AX_THROW_IF_FAILED_HR(device->CreateCommandQueue(&desc, IID_PPV_ARGS(&m_cmdQueue)), "Failed to create command queue");
		AX_CORE_LOG_TRACE("Successfully created command queue");
	}

	void D12CommandQueue::release() {
		m_cmdQueue.Reset();
	}

}
