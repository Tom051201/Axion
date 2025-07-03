#pragma once

namespace Axion {

	class D12CommandList {
	public:

		D12CommandList() = default;
		~D12CommandList();

		void initialize(ID3D12Device* device);
		void release();

		void reset();
		void close();

		inline ID3D12GraphicsCommandList* getCommandList() const { return m_cmdList.Get(); }
		inline ID3D12CommandAllocator* getCommandAllocator() const { return m_cmdAllocator.Get(); }

	private:

		Microsoft::WRL::ComPtr<ID3D12CommandAllocator> m_cmdAllocator;
		Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> m_cmdList;

	};

}
