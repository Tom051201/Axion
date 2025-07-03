#include "axpch.h"
#include "D12CommandList.h"

#include "AxionEngine/Source/core/Core.h"

namespace Axion {

	D12CommandList::~D12CommandList() {
		release();
	}

	void D12CommandList::initialize(ID3D12Device* device) {
		AX_THROW_IF_FAILED_HR(device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_cmdAllocator)), "Failed to create command allocator");
		AX_THROW_IF_FAILED_HR(device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_cmdAllocator.Get(), nullptr, IID_PPV_ARGS(&m_cmdList)), "Failed to create command list");
		AX_THROW_IF_FAILED_HR(m_cmdList->Close(), "Failed to close command list");
		AX_CORE_LOG_TRACE("Successfully created command list");

		#ifdef AX_DEBUG
		m_cmdAllocator->SetName(L"CommandAllocator");
		m_cmdList->SetName(L"CommandList");
		#endif
	}

	void D12CommandList::release() {
		m_cmdList.Reset();
		m_cmdAllocator.Reset();
	}

	void D12CommandList::reset() {
		AX_THROW_IF_FAILED_HR(m_cmdAllocator->Reset(), "Failed to reset command allocator");
		AX_THROW_IF_FAILED_HR(m_cmdList->Reset(m_cmdAllocator.Get(), nullptr), "Failed to reset command list");
	}

	void D12CommandList::close() {
		AX_THROW_IF_FAILED_HR(m_cmdList->Close(), "Failed to close command list");
	}

}
