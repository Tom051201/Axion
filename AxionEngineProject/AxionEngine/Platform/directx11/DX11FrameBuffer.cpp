#include "axpch.h"
#include "DX11FrameBuffer.h"

#include "AxionEngine/Platform/directx11/DX11Context.h"
#include "AxionEngine/Platform/directx11/DX11DebugLayer.h"

namespace Axion {

	namespace Utils {
		static DXGI_FORMAT ColorFormatToDXGI(ColorFormat format) {
			switch (format) {
			case ColorFormat::RGBA8: return DXGI_FORMAT_R8G8B8A8_UNORM;
			case ColorFormat::RED_INTEGER: return DXGI_FORMAT_R32_SINT;
			}
			return DXGI_FORMAT_R8G8B8A8_UNORM;
		}

		static DXGI_FORMAT DepthFormatToDXGI(DepthStencilFormat format) {
			switch (format) {
			case DepthStencilFormat::DEPTH32F: return DXGI_FORMAT_D32_FLOAT;
			case DepthStencilFormat::DEPTH24_STENCIL8: return DXGI_FORMAT_D24_UNORM_S8_UINT;
			}
			return DXGI_FORMAT_D32_FLOAT;
		}
	}

	DX11FrameBuffer::DX11FrameBuffer(const FrameBufferSpecification& spec) {
		m_context = static_cast<DX11Context*>(GraphicsContext::get()->getNativeContext());
		AX_CORE_ASSERT(m_context, "Failed to acquire DirectX11 context");

		try {
			resize(spec.width, spec.height);
			m_allocated = true;
		}
		catch (...) {
			AX_CORE_LOG_ERROR("Error creating DX11 frame buffer");
			throw;
		}
	}

	DX11FrameBuffer::~DX11FrameBuffer() {
		release();
	}

	void DX11FrameBuffer::release() {
		if (!m_allocated) return;

		m_colorTexture.Reset();
		m_colorRTV.Reset();
		m_colorSRV.Reset();

		m_depthTexture.Reset();
		m_depthDSV.Reset();

		m_entityIdTexture.Reset();
		m_entityIdRTV.Reset();
		m_readbackTexture.Reset();
	}

	void DX11FrameBuffer::resize(uint32_t width, uint32_t height) {
		width = std::max(1u, width);
		height = std::max(1u, height);

		release();
		m_specification.width = width;
		m_specification.height = height;

		auto* device = m_context->getDevice();
		HRESULT hr;

		// ----- Color Attachment -----
		D3D11_TEXTURE2D_DESC colorDesc = {};
		colorDesc.Width = width;
		colorDesc.Height = height;
		colorDesc.MipLevels = 1;
		colorDesc.ArraySize = 1;
		colorDesc.Format = Utils::ColorFormatToDXGI(m_specification.textureFormat);
		colorDesc.SampleDesc.Count = 1;
		colorDesc.Usage = D3D11_USAGE_DEFAULT;
		colorDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;

		hr = device->CreateTexture2D(&colorDesc, nullptr, &m_colorTexture);
		AX_THROW_IF_FAILED_HR(hr, "Failed to create DX11 FrameBuffer Color Texture");

		hr = device->CreateRenderTargetView(m_colorTexture.Get(), nullptr, &m_colorRTV);
		AX_THROW_IF_FAILED_HR(hr, "Failed to create DX11 FrameBuffer Color RTV");

		hr = device->CreateShaderResourceView(m_colorTexture.Get(), nullptr, &m_colorSRV);
		AX_THROW_IF_FAILED_HR(hr, "Failed to create DX11 FrameBuffer Color SRV");


		// ----- Optional Entity ID Attachment -----
		if (m_specification.useEntityIDAttachment) {
			D3D11_TEXTURE2D_DESC idDesc = colorDesc;
			idDesc.Format = DXGI_FORMAT_R32_SINT;
			idDesc.BindFlags = D3D11_BIND_RENDER_TARGET;

			hr = device->CreateTexture2D(&idDesc, nullptr, &m_entityIdTexture);
			AX_THROW_IF_FAILED_HR(hr, "Failed to create DX11 FrameBuffer EntityID Texture");

			hr = device->CreateRenderTargetView(m_entityIdTexture.Get(), nullptr, &m_entityIdRTV);
			AX_THROW_IF_FAILED_HR(hr, "Failed to create DX11 FrameBuffer EntityID RTV");

			// -- Create 1x1 Staging Texture For CPU Readback --
			D3D11_TEXTURE2D_DESC stageDesc = {};
			stageDesc.Width = 1;
			stageDesc.Height = 1;
			stageDesc.MipLevels = 1;
			stageDesc.ArraySize = 1;
			stageDesc.Format = DXGI_FORMAT_R32_SINT;
			stageDesc.SampleDesc.Count = 1;
			stageDesc.Usage = D3D11_USAGE_STAGING;
			stageDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
			stageDesc.BindFlags = 0;

			hr = device->CreateTexture2D(&stageDesc, nullptr, &m_readbackTexture);
			AX_THROW_IF_FAILED_HR(hr, "Failed to create DX11 FrameBuffer Readback Texture");
		}


		// ----- Depth Texture -----
		D3D11_TEXTURE2D_DESC depthDesc = {};
		depthDesc.Width = width;
		depthDesc.Height = height;
		depthDesc.MipLevels = 1;
		depthDesc.ArraySize = 1;
		depthDesc.Format = Utils::DepthFormatToDXGI(m_specification.depthStencilFormat);
		depthDesc.SampleDesc.Count = 1;
		depthDesc.Usage = D3D11_USAGE_DEFAULT;
		depthDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;

		hr = device->CreateTexture2D(&depthDesc, nullptr, &m_depthTexture);
		AX_THROW_IF_FAILED_HR(hr, "Failed to create DX11 FrameBuffer Depth Texture");

		hr = device->CreateDepthStencilView(m_depthTexture.Get(), nullptr, &m_depthDSV);
		AX_THROW_IF_FAILED_HR(hr, "Failed to create DX11 FrameBuffer Depth DSV");

		// ----- Debug Naming -----
		#ifdef AX_DEBUG
		DX11DebugLayer::setName(m_colorTexture.Get(), "DX11 FrameBuffer Color");
		DX11DebugLayer::setName(m_depthTexture.Get(), "DX11 FrameBuffer Depth");
		if (m_specification.useEntityIDAttachment) {
			DX11DebugLayer::setName(m_entityIdTexture.Get(), "DX11 FrameBuffer EntityID");
			DX11DebugLayer::setName(m_readbackTexture.Get(), "DX11 FrameBuffer Readback");
		}
		#endif
	}

	void DX11FrameBuffer::bind() const {
		auto* ctx = m_context->getDeviceContext();

		ID3D11RenderTargetView* rtvs[2] = { m_colorRTV.Get(), nullptr };
		uint32_t numTargets = 1;

		if (m_specification.useEntityIDAttachment) {
			rtvs[1] = m_entityIdRTV.Get();
			numTargets = 2;
		}

		ctx->OMSetRenderTargets(numTargets, rtvs, m_depthDSV.Get());

		D3D11_VIEWPORT vp = {};
		vp.Width = (float)m_specification.width;
		vp.Height = (float)m_specification.height;
		vp.MinDepth = 0.0f;
		vp.MaxDepth = 1.0f;
		ctx->RSSetViewports(1, &vp);
	}

	void DX11FrameBuffer::unbind() const {
		auto* ctx = m_context->getDeviceContext();
		ID3D11RenderTargetView* nullRTVs[2] = { nullptr, nullptr };
		ctx->OMSetRenderTargets(2, nullRTVs, nullptr);
	}

	void DX11FrameBuffer::clear() {
		clear(m_specification.clearColor);
	}

	void DX11FrameBuffer::clear(const Vec4 & clearColor) {
		auto* ctx = m_context->getDeviceContext();

		float color[4] = { clearColor.x, clearColor.y, clearColor.z, clearColor.w };
		ctx->ClearRenderTargetView(m_colorRTV.Get(), color);
		ctx->ClearDepthStencilView(m_depthDSV.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);

		if (m_specification.useEntityIDAttachment && m_entityIdRTV) {
			int clearID = -1;
			float idColor[4] = { reinterpret_cast<float&>(clearID), 0.0f, 0.0f, 0.0f };
			ctx->ClearRenderTargetView(m_entityIdRTV.Get(), idColor);
		}
	}

	void DX11FrameBuffer::clearDepth() {
		auto* ctx = m_context->getDeviceContext();
		ctx->ClearDepthStencilView(m_depthDSV.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);
	}

	void DX11FrameBuffer::clearAttachment(uint32_t attachmentIndex, int value) {
		if (attachmentIndex == 1 && m_specification.useEntityIDAttachment && m_entityIdRTV) {
			auto* ctx = m_context->getDeviceContext();
			float clearVal = reinterpret_cast<float&>(value);
			float clearColor[4] = { clearVal, 0.0f, 0.0f, 0.0f };
			ctx->ClearRenderTargetView(m_entityIdRTV.Get(), clearColor);
		}
	}

	int DX11FrameBuffer::readPixel(uint32_t attachmentIndex, int x, int y) {
		if (attachmentIndex != 1 || !m_specification.useEntityIDAttachment) return -1;
		auto* ctx = m_context->getDeviceContext();

		// -- Read Pixel Data From Previous Frame --
		int entityID = -1;
		D3D11_MAPPED_SUBRESOURCE mapped;
		if (SUCCEEDED(ctx->Map(m_readbackTexture.Get(), 0, D3D11_MAP_READ, 0, &mapped))) {
			int* data = reinterpret_cast<int*>(mapped.pData);
			entityID = data[0];
			ctx->Unmap(m_readbackTexture.Get(), 0);
		}

		// -- Queue New Copy Command For This Frame --
		if (x >= 0 && y >= 0 && x < (int)m_specification.width && y < (int)m_specification.height) {
			D3D11_BOX box;
			box.left = x;
			box.right = x + 1;
			box.top = y;
			box.bottom = y + 1;
			box.front = 0;
			box.back = 1;

			ctx->CopySubresourceRegion(
				m_readbackTexture.Get(), 0,
				0, 0, 0,
				m_entityIdTexture.Get(), 0,
				&box
			);
		}

		return entityID;
	}

}
