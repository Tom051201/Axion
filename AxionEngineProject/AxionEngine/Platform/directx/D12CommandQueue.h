#pragma once

namespace Axion {
	
	class D12CommandQueue {
	public:

		D12CommandQueue() = default;
		~D12CommandQueue();

		void initialize(ID3D12Device* device);
		void release();

		void executeCommandList(ID3D12CommandList* cmdList);
		void executeCommandLists(const std::vector<ID3D12CommandList*>& cmdLists);

		inline ID3D12CommandQueue* getCommandQueue() const { return m_cmdQueue.Get(); }

	private:

		Microsoft::WRL::ComPtr<ID3D12CommandQueue> m_cmdQueue;
	
	};

}
