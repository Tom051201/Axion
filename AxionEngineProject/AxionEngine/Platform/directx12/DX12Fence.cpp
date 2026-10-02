#include "axpch.h"
#include "DX12Fence.h"

#include "AxionEngine/Source/core/Core.h"
#include "AxionEngine/Source/graphics/GraphicsContext.h"

#include "AxionEngine/Platform/directx12/DX12CommandQueue.h"

namespace Axion {

	DX12Fence::~DX12Fence() {
		release();
	}

	void DX12Fence::initialize(ID3D12Device* device) {
		AX_THROW_IF_FAILED_HR(device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_fence)), "Failed to create fence");
		AX_CORE_LOG_TRACE("Successfully created fence");
		m_fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
		if (!m_fenceEvent) {
			AX_CORE_LOG_ERROR("Failed CreateEvent (fence)");
			throw std::runtime_error("CreateEvent failed");
		}

		#ifdef AX_DEBUG
		m_fence->SetName(L"Fence");
		#endif
	}

	void DX12Fence::release() {
		m_fence.Reset();

		CloseHandle(m_fenceEvent);
		m_fenceEvent = nullptr;
	}

	void DX12Fence::signal(ID3D12CommandQueue* queue, UINT64 fenceValue) {
		AX_THROW_IF_FAILED_HR(queue->Signal(m_fence.Get(), fenceValue), "Failed to signal fence");
	}

	void DX12Fence::wait(UINT64 fenceValue) {
		if (m_fence->GetCompletedValue() < fenceValue) {
			AX_THROW_IF_FAILED_HR(m_fence->SetEventOnCompletion(fenceValue, m_fenceEvent), "Failed SetEventOnCompletion");
			WaitForSingleObject(m_fenceEvent, INFINITE);
		}
	}

	bool DX12Fence::hasCompleted(UINT64 fenceValue) const {
		return m_fence->GetCompletedValue() >= fenceValue;
	}

}

