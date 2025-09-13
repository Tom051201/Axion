#pragma once
#include "axpch.h"

namespace Axion {

	class D12Device {
	public:

		D12Device() = default;
		~D12Device();

		void initialize();
		void release();

		std::string getAdapterName() const;

		ID3D12Device* getDevice() const { return m_device.Get(); }
		IDXGIFactory6* getFactory() const { return m_factory.Get(); }
		IDXGIAdapter1* getAdapter() const { return m_adapter.Get(); }

	private:

		Microsoft::WRL::ComPtr<ID3D12Device> m_device;
		Microsoft::WRL::ComPtr<IDXGIFactory6> m_factory;
		Microsoft::WRL::ComPtr<IDXGIAdapter1> m_adapter;

	};

}
