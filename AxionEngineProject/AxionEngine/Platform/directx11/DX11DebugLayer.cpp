#include "axpch.h"
#include "DX11DebugLayer.h"

#include <d3d11sdklayers.h>

#include "AxionEngine/Source/EngineConfig.h"

namespace Axion {

	void DX11DebugLayer::initialize(ID3D11Device* device) {
		if (!device) return;

		Microsoft::WRL::ComPtr<ID3D11InfoQueue> infoQueue;
		if (SUCCEEDED(device->QueryInterface(IID_PPV_ARGS(&infoQueue)))) {

			// -- Set Breaks On Errors And Corruptions --
			infoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_CORRUPTION, TRUE);
			infoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_ERROR, TRUE);

			// -- Optional Break On Warnings --
			if constexpr (Config::D11EnableDebugSeverityWarning) {
				infoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_WARNING, TRUE);
			}

			// -- Filter Out Harmless Warnings --
			D3D11_MESSAGE_ID hide[] = {
				D3D11_MESSAGE_ID_DEVICE_DRAW_RENDERTARGETVIEW_NOT_SET,
			};

			D3D11_INFO_QUEUE_FILTER filter = {};
			filter.DenyList.NumIDs = _countof(hide);
			filter.DenyList.pIDList = hide;
			infoQueue->AddStorageFilterEntries(&filter);

			AX_CORE_LOG_TRACE("DX11 Info Queue enabled and configured");
		}
	}

	void DX11DebugLayer::reportLiveObjects(ID3D11Device* device) {
		if (!device) return;

		Microsoft::WRL::ComPtr<ID3D11Debug> debugDevice;
		if (SUCCEEDED(device->QueryInterface(IID_PPV_ARGS(&debugDevice)))) {
			debugDevice->ReportLiveDeviceObjects(D3D11_RLDO_DETAIL | D3D11_RLDO_IGNORE_INTERNAL);
		}
	}

	void DX11DebugLayer::setName(ID3D11DeviceChild* object, const std::string& name) {
		if (object && !name.empty()) {
			object->SetPrivateData(WKPDID_D3DDebugObjectName, (UINT)name.size(), name.c_str());
		}
	}

}
