#include "axpch.h"
#include "DX12RenderContext.h"

#include <d3dx12/d3dx12.h>

#include "AxionEngine/Source/graphics/FrameBuffer.h"

namespace Axion {

	static D3D12_RESOURCE_STATES MapStateToDX12(ResourceState state) {
		switch (state) {
			case ResourceState::RenderTarget:			return D3D12_RESOURCE_STATE_RENDER_TARGET;
			case ResourceState::DepthWrite:				return D3D12_RESOURCE_STATE_DEPTH_WRITE;
			case ResourceState::DepthRead:				return D3D12_RESOURCE_STATE_DEPTH_READ;
			case ResourceState::PixelShaderResource:	return D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
			case ResourceState::CopyDest:				return D3D12_RESOURCE_STATE_COPY_DEST;
			case ResourceState::CopySource:				return D3D12_RESOURCE_STATE_COPY_SOURCE;
			case ResourceState::Present:				return D3D12_RESOURCE_STATE_PRESENT;
			case ResourceState::Common:
			default:									return D3D12_RESOURCE_STATE_COMMON;
		}
	}

	void DX12RenderContext::transitionResource(FrameBuffer* framebuffer, ResourceState beforeState, ResourceState afterState, bool isDepth) {
		if (!framebuffer) return;

		ID3D12Resource* nativeRes = nullptr;
		if (isDepth) {
			nativeRes = static_cast<ID3D12Resource*>(framebuffer->getNativeDepthResource());
		}
		else {
			nativeRes = static_cast<ID3D12Resource*>(framebuffer->getNativeColorResource());
		}

		if (nativeRes) {
			auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(
				nativeRes,
				MapStateToDX12(beforeState),
				MapStateToDX12(afterState)
			);
			m_commandList->ResourceBarrier(1, &barrier);
		}
	}

	void DX12RenderContext::setTextureHandle(uint32_t slot, void* handle) {
		D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle;
		gpuHandle.ptr = reinterpret_cast<UINT64>(handle);
		m_commandList->SetGraphicsRootDescriptorTable(slot, gpuHandle);
	}

	void DX12RenderContext::unbindBuffers() {
		m_commandList->IASetVertexBuffers(0, 0, nullptr);
		m_commandList->IASetIndexBuffer(nullptr);
	}

	void DX12RenderContext::draw(uint32_t vertexCount, uint32_t instanceCount, uint32_t startVertex, uint32_t startInstance) {
		m_commandList->DrawInstanced(vertexCount, instanceCount, startVertex, startInstance);
	}

}
