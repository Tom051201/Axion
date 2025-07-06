#include "axpch.h"
#include "D12DebugLayer.h"

#include "AxionEngine/Source/AxionSettings.h"

namespace Axion {

	void D12DebugLayer::initialize() {

		Microsoft::WRL::ComPtr<ID3D12Debug> debugController;
		if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)))) {
			debugController->EnableDebugLayer();
			AX_CORE_LOG_TRACE("D3D12 Debug Layer enabled");
		}

		Microsoft::WRL::ComPtr<IDXGIInfoQueue> dxgiInfoQueue;
		if (SUCCEEDED(DXGIGetDebugInterface1(0, IID_PPV_ARGS(&dxgiInfoQueue)))) {
			dxgiInfoQueue->SetBreakOnSeverity(DXGI_DEBUG_ALL, DXGI_INFO_QUEUE_MESSAGE_SEVERITY_ERROR, TRUE);
			dxgiInfoQueue->SetBreakOnSeverity(DXGI_DEBUG_ALL, DXGI_INFO_QUEUE_MESSAGE_SEVERITY_CORRUPTION, TRUE);
			#if AX_D12_ENABLE_DEBUG_SEVERITY_WARNING
			dxgiInfoQueue->SetBreakOnSeverity(DXGI_DEBUG_ALL, DXGI_INFO_QUEUE_MESSAGE_SEVERITY_WARNING, TRUE);
			#endif
			AX_CORE_LOG_TRACE("DXGI Debug Layer enabled");
		}

		#if AX_D12_ENABLE_DEBUG_GPU_BASED_VALIDATION
		Microsoft::WRL::ComPtr<ID3D12Debug1> debugController1;
		if (SUCCEEDED(debugController.As(&debugController1))) {
			debugController1->SetEnableGPUBasedValidation(TRUE);
			AX_CORE_LOG_TRACE("GPU-based validation enabled");
		}
		#endif

		Microsoft::WRL::ComPtr<ID3D12DeviceRemovedExtendedDataSettings> dredSettings;
		if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&dredSettings)))) {
			dredSettings->SetAutoBreadcrumbsEnablement(D3D12_DRED_ENABLEMENT_FORCED_ON);
			dredSettings->SetPageFaultEnablement(D3D12_DRED_ENABLEMENT_FORCED_ON);
			AX_CORE_LOG_TRACE("DRED enabled");
		}
		else {
			AX_CORE_LOG_WARN("DRED not supported on this system");
		}

	}

	void D12DebugLayer::reportLiveObjects() {

		Microsoft::WRL::ComPtr<IDXGIDebug1> dxgiDebug;
		if (SUCCEEDED(DXGIGetDebugInterface1(0, IID_PPV_ARGS(&dxgiDebug)))) {
			dxgiDebug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_DETAIL);
		}

	}

}
