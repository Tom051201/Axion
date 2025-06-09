#include "axpch.h"
#include "D12Fence.h"

#include "Axion/Core.h"

namespace Axion {

	D12Fence::~D12Fence() {
		release();
	}

	void D12Fence::initialize(ID3D12Device* device) {
		AX_THROW_IF_FAILED_HR(device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_fence)), "Failed to create fence");
		AX_CORE_LOG_TRACE("Successfully created fence");
		m_fenceValue = 1;
		m_fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
		if (!m_fenceEvent) {
			AX_CORE_LOG_ERROR("Failed CreateEvent (fence)");
			throw std::runtime_error("CreateEvent failed");
		} else {
			AX_CORE_LOG_TRACE("Successful CreateEvent (fence)");
		}
	}

	void D12Fence::release() {
		if (m_fence.Get()) { m_fence.Reset(); }
		if (m_fenceEvent) {
			CloseHandle(m_fenceEvent);
			m_fenceEvent = nullptr;
		}
	}

}

