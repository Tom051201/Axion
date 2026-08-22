#pragma once

#include <cstdint>
#include <filesystem>
#include <array>
#include <d3d11.h>
#include <wrl/client.h>

#include "AxionEngine/Source/graphics/Texture.h"

namespace Axion {

	////////////////////////////////////////////////////////////////////////////////
	///// DX11Texture2D ////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////////

	class DX11Texture2D : public Texture2D {
	public:

		DX11Texture2D(const std::filesystem::path& path);
		DX11Texture2D(uint32_t width, uint32_t height, void* data);
		DX11Texture2D(const uint8_t* data, size_t size);
		~DX11Texture2D() override;

		void release() override;

		void bind(uint32_t slot = 0) const override;
		void unbind() const override;

		uint32_t getWidth() const override { return m_width; }
		uint32_t getHeight() const override { return m_height; }

		void* getHandle() const override { return (void*)m_srv.Get(); }
		uint32_t getSrvHeapIndex() const override { return 0; }
		void* getNativeResource() const override { return (void*)m_textureResource.Get(); }

	private:

		uint32_t m_width = 0;
		uint32_t m_height = 0;
		uint32_t m_pixelSize = 4;

		Microsoft::WRL::ComPtr<ID3D11Texture2D> m_textureResource;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_srv;

	};

	////////////////////////////////////////////////////////////////////////////////
	///// DX11TextureCube //////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////////

	class DX11TextureCube : public TextureCube {
	public:

		DX11TextureCube(const std::filesystem::path& filePath);
		DX11TextureCube(const std::array<std::filesystem::path, 6>& paths);
		DX11TextureCube(const uint8_t* data, size_t size);
		~DX11TextureCube() override;

		void release() override;

		void bind(uint32_t slot = 0) const override;
		void unbind() const override;

		void* getHandle() const override { return (void*)m_srv.Get(); }
		uint32_t getFaceWidth() const override { return m_faceWidth; }
		uint32_t getFaceHeight() const override { return m_faceHeight; }

	private:

		uint32_t m_faceWidth = 0;
		uint32_t m_faceHeight = 0;
		uint32_t m_pixelSize = 4;

		Microsoft::WRL::ComPtr<ID3D11Texture2D> m_textureResource;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_srv;

		void setupGpuResources(const std::array<uint8_t*, 6>& pixels);

	};

	////////////////////////////////////////////////////////////////////////////////
	///// DX11DepthTexture /////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////////

	class DX11DepthTexture : public Texture2D {
	public:

		DX11DepthTexture(uint32_t width, uint32_t height);
		~DX11DepthTexture() override;

		void release() override;

		void bind(uint32_t slot = 0) const override;
		void unbind() const override;

		uint32_t getWidth() const override { return m_width; }
		uint32_t getHeight() const override { return m_height; }

		void* getHandle() const override { return (void*)m_srv.Get(); }
		uint32_t getSrvHeapIndex() const override { return 0; }
		void* getNativeResource() const override { return (void*)m_textureResource.Get(); }

		ID3D11DepthStencilView* getDsv() const { return m_dsv.Get(); }

	private:

		uint32_t m_width = 0;
		uint32_t m_height = 0;

		Microsoft::WRL::ComPtr<ID3D11Texture2D> m_textureResource;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_srv;
		Microsoft::WRL::ComPtr<ID3D11DepthStencilView> m_dsv;

	};

}
