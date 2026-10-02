#pragma once
#include "AxionEngine/Source/graphics/RenderContext.h"

#include <d3d12.h>
#include <wrl/client.h>

namespace Axion {

	class DX12RenderContext : public RenderContext {
	public:

		DX12RenderContext(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> cmdList)
			: m_commandList(cmdList) {}

		void* getNativeCommandList() const override { return m_commandList.Get(); }
		ID3D12GraphicsCommandList* getCmdList() const { return m_commandList.Get(); }

		void begin() override {}

		void end() override {
			m_commandList->Close();
		}

	private:

		Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> m_commandList;

	};

}
