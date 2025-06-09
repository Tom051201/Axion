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
				AX_CORE_LOG_TRACE("Successfully created device for adapter");
				return;
			}
		}

		AX_THROW_IF_FAILED_HR(D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&m_device)), "Failed to create device");
		AX_CORE_LOG_TRACE("Successfully created device for WARP");
	}

	void D12Device::release() {
		m_device.Reset();
		m_adapter.Reset();
		m_factory.Reset();
	}

}


