#include "axpch.h"
#include "D12Device.h"

#include "Axion/Core.h"

namespace Axion {

	D12Device::~D12Device() {
		release();
	}

	void D12Device::initialize() {
		
		AX_THROW_IF_FAILED_HR(CreateDXGIFactory1(IID_PPV_ARGS(&m_factory)), "Failed to create Factory1");
		
		for (UINT i = 0; m_factory->EnumAdapters1(i, &m_adapter) != DXGI_ERROR_NOT_FOUND; ++i) {
			DXGI_ADAPTER_DESC1 desc;
			m_adapter->GetDesc1(&desc);
			if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) continue;
			if (SUCCEEDED(D3D12CreateDevice(m_adapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&m_device)))) {
				AX_CORE_LOG_INFO("Successfully created device for adapter");
				return;
			}
		}

		#ifdef AX_DEBUG
		if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&m_debugController)))) {
			m_debugController->EnableDebugLayer();
		}
		if (SUCCEEDED(DXGIGetDebugInterface1(0, IID_PPV_ARGS(&m_dxgiInfoQueue)))) {
			m_dxgiInfoQueue->SetBreakOnSeverity(DXGI_DEBUG_ALL, DXGI_INFO_QUEUE_MESSAGE_SEVERITY_ERROR, TRUE);
			m_dxgiInfoQueue->SetBreakOnSeverity(DXGI_DEBUG_ALL, DXGI_INFO_QUEUE_MESSAGE_SEVERITY_CORRUPTION, TRUE);
		}
		#endif

		AX_THROW_IF_FAILED_HR(D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&m_device)), "Failed to create device");
		AX_CORE_LOG_INFO("Successfully created device for WARP");
	}

	void D12Device::release() {
		if (m_device.Get()) { m_device.Reset(); }
		if (m_factory.Get()) { m_factory.Reset(); }
		if (m_adapter.Get()) { m_adapter.Reset(); }
		if (m_debugController.Get()) { m_debugController.Reset(); }
		if (m_dxgiInfoQueue.Get()) { m_dxgiInfoQueue.Reset(); }
	}

}


