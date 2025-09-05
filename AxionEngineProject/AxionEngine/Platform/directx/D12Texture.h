#pragma once
#include "axpch.h"

#include "AxionEngine/Source/render/Texture.h"

#include "AxionEngine/Vendor/stb_image/stb_image.h"

namespace Axion {


	////////////////////////////////////////////////////////////////////////////////
	///// D12Texture2D /////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////////


	class D12Texture2D : public Texture2D {
	public:

		D12Texture2D(const std::string& path);
		~D12Texture2D() override;

		void release() override;

		void bind() const override;
		void unbind() const override;

		void* getHandle() const override;

		uint32_t getWidth() const override { return m_width; }
		uint32_t getHeight() const override { return m_height; }

		uint32_t getSrvHeapIndex() const { return m_srvHeapIndex; }

	private:

		uint32_t m_width = 0;
		uint32_t m_height = 0;
		uint32_t m_pixelSize = 4;

		Microsoft::WRL::ComPtr<ID3D12Resource> m_textureResource;
		Microsoft::WRL::ComPtr<ID3D12Resource> m_uploadHeap;
		uint32_t m_srvHeapIndex;
	};


	////////////////////////////////////////////////////////////////////////////////
	///// D12TextureCube ///////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////////


	class D12TextureCube : public TextureCube {
	public:

		// Order: +X, -X, +Y, -Y, +Z, -Z
		D12TextureCube(const std::string& filePath);
		D12TextureCube(const std::array<std::string, 6>& paths);
		~D12TextureCube() override;

		void release() override;

		void bind() const override;
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

}
