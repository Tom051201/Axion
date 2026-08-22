#pragma once

#include <cstdint>
#include <d3d11.h>
#include <wrl/client.h>

#include "AxionEngine/Source/graphics/FrameBuffer.h"

namespace Axion {

	class DX11Context;

	class DX11FrameBuffer : public FrameBuffer {
	public:

		DX11FrameBuffer(const FrameBufferSpecification& spec);
		~DX11FrameBuffer() override;

		void release() override;
		void resize(uint32_t width, uint32_t height) override;

		void bind() const override;
		void unbind() const override;

		void clear() override;
		void clear(const Vec4& clearColor) override;
		void clearDepth() override;

		void clearAttachment(uint32_t attachmentIndex, int value) override;
		int readPixel(uint32_t attachmentIndex, int x, int y) override;

		void* getColorAttachmentHandle() const override { return (void*)m_colorSRV.Get(); }
		void* getColorAttachmentNativeResource() const override { return (void*)m_colorTexture.Get(); }
		const FrameBufferSpecification& getSpecification() const override { return m_specification; }

	private:

		DX11Context* m_context = nullptr;
		FrameBufferSpecification m_specification;
		bool m_allocated = false;

		Microsoft::WRL::ComPtr<ID3D11Texture2D> m_colorTexture;
		Microsoft::WRL::ComPtr<ID3D11RenderTargetView> m_colorRTV;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_colorSRV;

		Microsoft::WRL::ComPtr<ID3D11Texture2D> m_depthTexture;
		Microsoft::WRL::ComPtr<ID3D11DepthStencilView> m_depthDSV;

		// -- Entity ID Resources --
		Microsoft::WRL::ComPtr<ID3D11Texture2D> m_entityIdTexture;
		Microsoft::WRL::ComPtr<ID3D11RenderTargetView> m_entityIdRTV;
		Microsoft::WRL::ComPtr<ID3D11Texture2D> m_readbackTexture;

	};

}
