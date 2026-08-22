#include "axpch.h"
#include "DX11Texture.h"

#include <stb_image/stb_image.h>

#include "AxionEngine/Platform/directx11/DX11Context.h"
#include "AxionEngine/Platform/directx11/DX11DebugLayer.h"

#include "AxionEngine/Source/graphics/GraphicsContext.h"

namespace Axion {

	////////////////////////////////////////////////////////////////////////////////
	///// DX11Texture2D ////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////////

	DX11Texture2D::DX11Texture2D(const std::filesystem::path& path) {
		auto* device = static_cast<DX11Context*>(GraphicsContext::get()->getNativeContext())->getDevice();

		int texWidth, texHeight, texChannels;
		stbi_set_flip_vertically_on_load(false);
		stbi_uc* pixels = stbi_load(path.string().c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
		AX_CORE_ASSERT(pixels, "Failed to load texture image: " + path.string());

		m_width = static_cast<uint32_t>(texWidth);
		m_height = static_cast<uint32_t>(texHeight);

		D3D11_TEXTURE2D_DESC desc = {};
		desc.Width = m_width;
		desc.Height = m_height;
		desc.MipLevels = 1;
		desc.ArraySize = 1;
		desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		desc.SampleDesc.Count = 1;
		desc.Usage = D3D11_USAGE_IMMUTABLE;
		desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

		D3D11_SUBRESOURCE_DATA initData = {};
		initData.pSysMem = pixels;
		initData.SysMemPitch = m_width * m_pixelSize;

		HRESULT hr = device->CreateTexture2D(&desc, &initData, &m_textureResource);
		AX_THROW_IF_FAILED_HR(hr, "Failed to create DX11 Texture2D");

		hr = device->CreateShaderResourceView(m_textureResource.Get(), nullptr, &m_srv);
		AX_THROW_IF_FAILED_HR(hr, "Failed to create DX11 Texture2D SRV");

		stbi_image_free(pixels);

		#ifdef AX_DEBUG
		DX11DebugLayer::setName(m_textureResource.Get(), "DX11 Texture2D");
		DX11DebugLayer::setName(m_srv.Get(), "DX11 Texture2D SRV");
		#endif
	}

	DX11Texture2D::DX11Texture2D(uint32_t width, uint32_t height, void* data)
		: m_width(width), m_height(height) {

		auto* device = static_cast<DX11Context*>(GraphicsContext::get()->getNativeContext())->getDevice();

		bool generatedData = false;
		if (!data) {
			generatedData = true;
			uint32_t* whiteData = new uint32_t[width * height];
			for (uint32_t i = 0; i < width * height; i++) whiteData[i] = 0xFFFFFFFF;
			data = whiteData;
		}

		D3D11_TEXTURE2D_DESC desc = {};
		desc.Width = m_width;
		desc.Height = m_height;
		desc.MipLevels = 1;
		desc.ArraySize = 1;
		desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		desc.SampleDesc.Count = 1;
		desc.Usage = D3D11_USAGE_IMMUTABLE;
		desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

		D3D11_SUBRESOURCE_DATA initData = {};
		initData.pSysMem = data;
		initData.SysMemPitch = m_width * m_pixelSize;

		HRESULT hr = device->CreateTexture2D(&desc, &initData, &m_textureResource);
		AX_THROW_IF_FAILED_HR(hr, "Failed to create DX11 Texture2D");

		hr = device->CreateShaderResourceView(m_textureResource.Get(), nullptr, &m_srv);
		AX_THROW_IF_FAILED_HR(hr, "Failed to create DX11 Texture2D SRV");

		if (generatedData) {
			delete[] static_cast<uint32_t*>(data);
		}

		#ifdef AX_DEBUG
		DX11DebugLayer::setName(m_textureResource.Get(), "DX11 Texture2D");
		DX11DebugLayer::setName(m_srv.Get(), "DX11 Texture2D SRV");
		#endif
	}

	DX11Texture2D::DX11Texture2D(const uint8_t* data, size_t size) {
		auto* device = static_cast<DX11Context*>(GraphicsContext::get()->getNativeContext())->getDevice();

		int texWidth, texHeight, texChannels;
		stbi_set_flip_vertically_on_load(false);
		stbi_uc* pixels = stbi_load_from_memory(data, static_cast<int>(size), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
		AX_CORE_ASSERT(pixels, "Failed to load Texture2D from memory buffer!");

		m_width = static_cast<uint32_t>(texWidth);
		m_height = static_cast<uint32_t>(texHeight);

		D3D11_TEXTURE2D_DESC desc = {};
		desc.Width = m_width;
		desc.Height = m_height;
		desc.MipLevels = 1;
		desc.ArraySize = 1;
		desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		desc.SampleDesc.Count = 1;
		desc.Usage = D3D11_USAGE_IMMUTABLE;
		desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

		D3D11_SUBRESOURCE_DATA initData = {};
		initData.pSysMem = pixels;
		initData.SysMemPitch = m_width * m_pixelSize;

		HRESULT hr = device->CreateTexture2D(&desc, &initData, &m_textureResource);
		AX_THROW_IF_FAILED_HR(hr, "Failed to create DX11 Texture2D");

		hr = device->CreateShaderResourceView(m_textureResource.Get(), nullptr, &m_srv);
		AX_THROW_IF_FAILED_HR(hr, "Failed to create DX11 Texture2D SRV");

		stbi_image_free(pixels);

		#ifdef AX_DEBUG
		DX11DebugLayer::setName(m_textureResource.Get(), "DX11 Texture2D");
		DX11DebugLayer::setName(m_srv.Get(), "DX11 Texture2D SRV");
		#endif
	}

	DX11Texture2D::~DX11Texture2D() {
		release();
	}

	void DX11Texture2D::release() {
		m_textureResource.Reset();
		m_srv.Reset();
	}

	void DX11Texture2D::bind(uint32_t slot) const {
		auto* ctx = static_cast<DX11Context*>(GraphicsContext::get()->getNativeContext())->getDeviceContext();
		// Ignore DX12 'slot' and map directly to DX11 register t0
		ctx->PSSetShaderResources(0, 1, m_srv.GetAddressOf());
	}

	void DX11Texture2D::unbind() const {}

	////////////////////////////////////////////////////////////////////////////////
	///// DX11TextureCube //////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////////

	DX11TextureCube::DX11TextureCube(const std::filesystem::path& filePath) {
		int texWidth, texHeight, texChannels;
		stbi_set_flip_vertically_on_load(false);
		stbi_uc* fullImage = stbi_load(filePath.string().c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
		AX_CORE_ASSERT(fullImage, "Failed to load cubemap image");

		m_faceWidth = texWidth / 4;
		m_faceHeight = texHeight / 3;

		std::array<std::pair<int, int>, 6> faceOffsets = {
			std::make_pair(2, 1), std::make_pair(0, 1), std::make_pair(1, 0),
			std::make_pair(1, 2), std::make_pair(1, 1), std::make_pair(3, 1)
		};

		std::array<uint8_t*, 6> pixels;
		for (int i = 0; i < 6; i++) {
			pixels[i] = new stbi_uc[m_faceWidth * m_faceHeight * m_pixelSize];
			int offsetX = faceOffsets[i].first * m_faceWidth;
			int offsetY = faceOffsets[i].second * m_faceHeight;

			for (uint32_t y = 0; y < m_faceHeight; y++) {
				memcpy(pixels[i] + y * m_faceWidth * m_pixelSize, fullImage + ((offsetY + y) * texWidth + offsetX) * m_pixelSize, m_faceWidth * m_pixelSize);
			}
		}

		stbi_image_free(fullImage);
		setupGpuResources(pixels);
		for (int i = 0; i < 6; i++) delete[] pixels[i];
	}

	DX11TextureCube::DX11TextureCube(const std::array<std::filesystem::path, 6>& paths) {
		int texWidth, texHeight, texChannels;
		stbi_set_flip_vertically_on_load(false);
		std::array<uint8_t*, 6> pixels = {};

		for (int i = 0; i < 6; i++) {
			pixels[i] = stbi_load(paths[i].string().c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
			AX_CORE_ASSERT(pixels[i], "Failed to load cubemap face");
		}

		m_faceWidth = static_cast<uint32_t>(texWidth);
		m_faceHeight = static_cast<uint32_t>(texHeight);

		setupGpuResources(pixels);
		for (int i = 0; i < 6; i++) stbi_image_free(pixels[i]);
	}

	DX11TextureCube::DX11TextureCube(const uint8_t* data, size_t size) {
		int texWidth, texHeight, texChannels;
		stbi_set_flip_vertically_on_load(false);
		stbi_uc* fullImage = stbi_load_from_memory(data, static_cast<int>(size), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);

		m_faceWidth = texWidth / 4;
		m_faceHeight = texHeight / 3;

		std::array<std::pair<int, int>, 6> faceOffsets = {
			std::make_pair(2, 1), std::make_pair(0, 1), std::make_pair(1, 0),
			std::make_pair(1, 2), std::make_pair(1, 1), std::make_pair(3, 1)
		};

		std::array<uint8_t*, 6> pixels;
		for (int i = 0; i < 6; i++) {
			pixels[i] = new stbi_uc[m_faceWidth * m_faceHeight * m_pixelSize];
			int offsetX = faceOffsets[i].first * m_faceWidth;
			int offsetY = faceOffsets[i].second * m_faceHeight;

			for (uint32_t y = 0; y < m_faceHeight; y++) {
				memcpy(pixels[i] + y * m_faceWidth * m_pixelSize, fullImage + ((offsetY + y) * texWidth + offsetX) * m_pixelSize, m_faceWidth * m_pixelSize);
			}
		}

		stbi_image_free(fullImage);
		setupGpuResources(pixels);
		for (int i = 0; i < 6; i++) delete[] pixels[i];
	}

	DX11TextureCube::~DX11TextureCube() {
		release();
	}

	void DX11TextureCube::release() {
		m_textureResource.Reset();
		m_srv.Reset();
	}

	void DX11TextureCube::bind(uint32_t slot) const {
		auto* ctx = static_cast<DX11Context*>(GraphicsContext::get()->getNativeContext())->getDeviceContext();
		// Ignore DX12 'slot' and map directly to DX11 register t0
		ctx->PSSetShaderResources(0, 1, m_srv.GetAddressOf());
	}

	void DX11TextureCube::unbind() const {}

	void DX11TextureCube::setupGpuResources(const std::array<uint8_t*, 6>&pixels) {
		auto* device = static_cast<DX11Context*>(GraphicsContext::get()->getNativeContext())->getDevice();

		D3D11_TEXTURE2D_DESC desc = {};
		desc.Width = m_faceWidth;
		desc.Height = m_faceHeight;
		desc.MipLevels = 1;
		desc.ArraySize = 6;
		desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		desc.SampleDesc.Count = 1;
		desc.Usage = D3D11_USAGE_IMMUTABLE;
		desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		desc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;

		std::array<D3D11_SUBRESOURCE_DATA, 6> initData = {};
		for (int i = 0; i < 6; i++) {
			initData[i].pSysMem = pixels[i];
			initData[i].SysMemPitch = m_faceWidth * m_pixelSize;
		}

		HRESULT hr = device->CreateTexture2D(&desc, initData.data(), &m_textureResource);
		AX_THROW_IF_FAILED_HR(hr, "Failed to create DX11 TextureCube");

		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.Format = desc.Format;
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
		srvDesc.TextureCube.MipLevels = 1;

		hr = device->CreateShaderResourceView(m_textureResource.Get(), &srvDesc, &m_srv);
		AX_THROW_IF_FAILED_HR(hr, "Failed to create DX11 TextureCube SRV");

		#ifdef AX_DEBUG
		DX11DebugLayer::setName(m_textureResource.Get(), "DX11 TextureCube");
		DX11DebugLayer::setName(m_srv.Get(), "DX11 TextureCube SRV");
		#endif
	}

	////////////////////////////////////////////////////////////////////////////////
	///// DX11DepthTexture /////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////////

	DX11DepthTexture::DX11DepthTexture(uint32_t width, uint32_t height)
		: m_width(width), m_height(height) {
		auto* device = static_cast<DX11Context*>(GraphicsContext::get()->getNativeContext())->getDevice();

		D3D11_TEXTURE2D_DESC desc = {};
		desc.Width = m_width;
		desc.Height = m_height;
		desc.MipLevels = 1;
		desc.ArraySize = 1;
		desc.Format = DXGI_FORMAT_R32_TYPELESS;
		desc.SampleDesc.Count = 1;
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
		HRESULT hr = device->CreateTexture2D(&desc, nullptr, &m_textureResource);
		AX_THROW_IF_FAILED_HR(hr, "Failed to create DX11 DepthTexture");

		D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
		dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
		dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
		hr = device->CreateDepthStencilView(m_textureResource.Get(), &dsvDesc, &m_dsv);
		AX_THROW_IF_FAILED_HR(hr, "Failed to create DX11 DepthTexture DSV");

		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.Format = DXGI_FORMAT_R32_FLOAT;
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MipLevels = 1;
		hr = device->CreateShaderResourceView(m_textureResource.Get(), &srvDesc, &m_srv);
		AX_THROW_IF_FAILED_HR(hr, "Failed to create DX11 DepthTexture SRV");

		#ifdef AX_DEBUG
		DX11DebugLayer::setName(m_textureResource.Get(), "DX11 DepthTexture");
		DX11DebugLayer::setName(m_dsv.Get(), "DX11 DepthTexture DSV");
		DX11DebugLayer::setName(m_srv.Get(), "DX11 DepthTexture SRV");
		#endif
	}

	DX11DepthTexture::~DX11DepthTexture() {
		release();
	}

	void DX11DepthTexture::release() {
		m_textureResource.Reset();
		m_srv.Reset();
		m_dsv.Reset();
	}

	void DX11DepthTexture::bind(uint32_t slot) const {
		auto* ctx = static_cast<DX11Context*>(GraphicsContext::get()->getNativeContext())->getDeviceContext();
		// Ignore DX12 'slot' and map directly to DX11 register t0
		ctx->PSSetShaderResources(0, 1, m_srv.GetAddressOf());
	}

	void DX11DepthTexture::unbind() const {}

}
