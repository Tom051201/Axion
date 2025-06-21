#include "axpch.h"
#include "D12Device.h"

#include "Axion/core/Core.h"
#include "Axion/core/Application.h"

namespace Axion {

	D12Device::~D12Device() {
		release();
	}

	void D12Device::initialize() {
		
		AX_THROW_IF_FAILED_HR(CreateDXGIFactory1(IID_PPV_ARGS(&m_factory)), "Failed to create Factory1");
		
		Microsoft::WRL::ComPtr<IDXGIAdapter1> adapter;
		for (UINT i = 0; m_factory->EnumAdapters1(i, &adapter) != DXGI_ERROR_NOT_FOUND; ++i) {
			DXGI_ADAPTER_DESC1 desc;
			adapter->GetDesc1(&desc);
			
			std::wstring ws(desc.Description);
			AX_CORE_LOG_TRACE("Attempting device creation on adapter : {0}", std::string(ws.begin(), ws.end()));
			AX_CORE_LOG_TRACE("VRAM: {0} MB", desc.DedicatedVideoMemory / (1024 * 1024));

			if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) continue;
			if (SUCCEEDED(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&m_device)))) {
				m_adapter = adapter;
				
				LARGE_INTEGER driverVersion;
				if (SUCCEEDED(adapter->CheckInterfaceSupport(__uuidof(IDXGIDevice), &driverVersion))) {
					WORD product = HIWORD(driverVersion.HighPart);
					WORD version = LOWORD(driverVersion.HighPart);
					WORD subVersion = HIWORD(driverVersion.LowPart);
					WORD build = LOWORD(driverVersion.LowPart);
					
					Application::get().getSystemInfo().gpuDriverVersion =
						std::to_string(product) + "." +
						std::to_string(version) + "." +
						std::to_string(subVersion) + "." +
						std::to_string(build);
				}

				Application::get().getSystemInfo().gpuName = std::string(ws.begin(), ws.end());
				Application::get().getSystemInfo().vramMB = desc.DedicatedVideoMemory / (1024 * 1024);
				AX_CORE_LOG_TRACE("Successfully created device for adapter");
				return;
			}
		}

		AX_CORE_LOG_WARN("Falling back to WARP adapter");
		AX_THROW_IF_FAILED_HR(D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&m_device)), "Failed to create device");
		Application::get().getSystemInfo().gpuName = "Unknown";
		Application::get().getSystemInfo().gpuDriverVersion = "Unknown";
		AX_CORE_LOG_TRACE("WARP device created successfully");
	}

	void D12Device::release() {
		m_device.Reset();
		m_adapter.Reset();
		m_factory.Reset();
	}

	std::string D12Device::getAdapterName() const {
		if (!m_device) return "Unknown";

		DXGI_ADAPTER_DESC1 desc;
		m_adapter->GetDesc1(&desc);
		std::wstring wdesc(desc.Description);
		return std::string(wdesc.begin(), wdesc.end());
	}

	

}
