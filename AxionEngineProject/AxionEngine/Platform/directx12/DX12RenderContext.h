#pragma once
#include "AxionEngine/Source/graphics/RenderContext.h"

#include <d3d12.h>
#include <wrl/client.h>

namespace Axion {

	class DX12RenderContext : public RenderContext {
	public:

		DX12RenderContext(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> cmdList)
			: m_commandList(cmdList) {}

		void begin() override {}

		void end() override {
			m_commandList->Close();
		}

		void transitionResource(FrameBuffer* framebuffer, ResourceState beforeState, ResourceState afterState, bool isDepth) override;

		void setTextureHandle(uint32_t slot, void* handle) override;
		void unbindBuffers() override;
		void draw(uint32_t vertexCount, uint32_t instanceCount = 1, uint32_t startVertex = 0, uint32_t startInstance = 0) override;

		ID3D12GraphicsCommandList* getCmdList() const { return m_commandList.Get(); }

	private:

		Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> m_commandList;

	};

}
