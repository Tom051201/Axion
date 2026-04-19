#include "axpch.h"
#include "DX12Fence.h"

#include "AxionEngine/Source/core/Core.h"
#include "AxionEngine/Source/render/GraphicsContext.h"

#include "AxionEngine/Platform/directx/DX12CommandQueue.h"

namespace Axion {

	DX12Fence::~DX12Fence() {
		release();
	}

	void DX12Fence::initialize(ID3D12Device* device) {
		AX_THROW_IF_FAILED_HR(device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_fence)), "Failed to create fence");
		AX_CORE_LOG_TRACE("Successfully created fence");
		m_fenceValue = 0;
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

	void DX12Fence::waitForGPU() {
		if (m_fence->GetCompletedValue() < m_fenceValue) {
			AX_THROW_IF_FAILED_HR(m_fence->SetEventOnCompletion(m_fenceValue, m_fenceEvent), "Failed SetEventOnCompletion (fence)");
			WaitForSingleObject(m_fenceEvent, INFINITE);
		}
	}

	void DX12Fence::signalAndWait() {
		auto* queue = static_cast<DX12CommandQueue*>(GraphicsContext::get()->getNativeContext())->getCommandQueue();
		
		AX_THROW_IF_FAILED_HR(queue->Signal(m_fence.Get(), m_fenceValue), "Failed to signal fence");
		waitForGPU();
		m_fenceValue++;
	}

	bool DX12Fence::hasCompleted() const {
		return m_fence->GetCompletedValue() >= m_fenceValue;
	}

}

