#pragma once
#include "axpch.h"

#include "AxionEngine/Source/render/Texture.h"

#include "AxionEngine/Vendor/stb_image/stb_image.h"

namespace Axion {


	////////////////////////////////////////////////////////////////////////////////
	///// DX12Texture2D ////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////////


	class DX12Texture2D : public Texture2D {
	public:

		DX12Texture2D(const std::filesystem::path& path);
		DX12Texture2D(uint32_t width, uint32_t height, void* data);
		DX12Texture2D(const uint8_t* data, size_t size);
		~DX12Texture2D() override;

		void release() override;

		void bind(uint32_t slot = 0) const override;
		void unbind() const override;

		uint32_t getWidth() const override { return m_width; }
		uint32_t getHeight() const override { return m_height; }

		void* getHandle() const override;
		uint32_t getSrvHeapIndex() const override { return m_srvHeapIndex; }


	private:

		uint32_t m_width = 0;
		uint32_t m_height = 0;
		uint32_t m_pixelSize = 4;

		Microsoft::WRL::ComPtr<ID3D12Resource> m_textureResource;
		Microsoft::WRL::ComPtr<ID3D12Resource> m_uploadHeap;
		uint32_t m_srvHeapIndex;
	};


	////////////////////////////////////////////////////////////////////////////////
	///// DX12TextureCube //////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////////


	class DX12TextureCube : public TextureCube {
	public:

		// Order: +X, -X, +Y, -Y, +Z, -Z
		DX12TextureCube(const std::filesystem::path& filePath);
		DX12TextureCube(const std::array<std::filesystem::path, 6>& paths);
		DX12TextureCube(const uint8_t* data, size_t size);
		~DX12TextureCube() override;

		void release() override;

		void bind(uint32_t slot = 0) const override;
		void unbind() const override;

		void* getHandle() const override;

		uint32_t getFaceWidth() const override { return m_faceWidth; }
		uint32_t getFaceHeight() const override { return m_faceHeight; }

		uint32_t getSrvHeapIndex() const { return m_srvHeapIndex; }

	private:

		uint32_t m_faceWidth = 0;
		uint32_t m_faceHeight = 0;
		uint32_t m_pixelSize = 4;

		Microsoft::WRL::ComPtr<ID3D12Resource> m_textureResource;
		Microsoft::WRL::ComPtr<ID3D12Resource> m_uploadHeap;
		uint32_t m_srvHeapIndex;


		void setupGpuResources(const std::array<stbi_uc*, 6>& pixels);

	};


	////////////////////////////////////////////////////////////////////////////////
	///// DX12DepthTexture /////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////////


	class DX12DepthTexture : public Texture2D {
	public:

		DX12DepthTexture(uint32_t width, uint32_t height);
		~DX12DepthTexture() override;

		void release() override;

		void bind(uint32_t slot = 0) const override;
		void unbind() const override;

		uint32_t getWidth() const override { return m_width; }
		uint32_t getHeight() const override { return m_height; }

		void* getHandle() const override;
		uint32_t getSrvHeapIndex() const override { return m_srvHeapIndex; }
		ID3D12Resource* getResource() const { return m_textureResource.Get(); }

		D3D12_CPU_DESCRIPTOR_HANDLE getDsvHandle() const { return m_dsvHandle; }

	private:

		uint32_t m_width = 0;
		uint32_t m_height = 0;

		Microsoft::WRL::ComPtr<ID3D12Resource> m_textureResource;

		uint32_t m_srvHeapIndex;

		D3D12_CPU_DESCRIPTOR_HANDLE m_dsvHandle;

	};

}
