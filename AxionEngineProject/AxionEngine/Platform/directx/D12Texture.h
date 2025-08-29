#pragma once
#include "axpch.h"

#include "AxionEngine/Source/render/Texture.h"

namespace Axion {

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

		uint32_t m_srvHeapIndex = 0;
	};

}
