#pragma once

namespace Axion {
	
	class DX12CommandQueue {
	public:

		DX12CommandQueue() = default;
		~DX12CommandQueue();

		void initialize(ID3D12Device* device);
		void release();

		void executeCommandList(ID3D12CommandList* cmdList);
		void executeCommandLists(const std::vector<ID3D12CommandList*>& cmdLists);

		inline ID3D12CommandQueue* getCommandQueue() const { return m_cmdQueue.Get(); }

	private:

		Microsoft::WRL::ComPtr<ID3D12CommandQueue> m_cmdQueue;
	
	};

}
