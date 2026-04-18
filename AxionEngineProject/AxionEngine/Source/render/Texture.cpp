#include "axpch.h"
#include "Texture.h"

#include "AxionEngine/Source/render/Renderer.h"

#include "AxionEngine/Platform/directx/D12Texture.h"

namespace Axion {

	Ref<Texture2D> Texture2D::create(const std::filesystem::path& path) {

		switch (Renderer::getAPI()) {

			case RendererAPI::None: { AX_CORE_ASSERT(false, "None is not supported yet!"); break; }
			case RendererAPI::DirectX12: { return std::make_shared<D12Texture2D>(path); }

		}

		return nullptr;
	}

	Ref<Texture2D> Texture2D::create(uint32_t width, uint32_t height, void* data) {

		switch (Renderer::getAPI()) {

			case RendererAPI::None: { AX_CORE_ASSERT(false, "None is not supported yet!"); break; }
			case RendererAPI::DirectX12: { return std::make_shared<D12Texture2D>(width, height, data); }

		}

		return nullptr;

	}

	Ref<Texture2D> Texture2D::create(const uint8_t* data, size_t size) {

		switch (Renderer::getAPI()) {

			case RendererAPI::None: { AX_CORE_ASSERT(false, "None is not supported yet!"); break; }
			case RendererAPI::DirectX12: { return std::make_shared<D12Texture2D>(data, size); }

		}

		return nullptr;

	}



	Ref<TextureCube> TextureCube::create(const std::array<std::filesystem::path, 6>& paths) {

		switch (Renderer::getAPI()) {

			case RendererAPI::None: { AX_CORE_ASSERT(false, "None is not supported yet!"); break; }
			case RendererAPI::DirectX12: { return std::make_shared<D12TextureCube>(paths); }

		}

		return nullptr;
	}

	Ref<TextureCube> TextureCube::create(const std::filesystem::path& filePath) {

		switch (Renderer::getAPI()) {

			case RendererAPI::None: { AX_CORE_ASSERT(false, "None is not supported yet!"); break; }
			case RendererAPI::DirectX12: { return std::make_shared<D12TextureCube>(filePath); }

		}

		return nullptr;
	}

	Ref<TextureCube> TextureCube::create(const uint8_t* data, size_t size) {

		switch (Renderer::getAPI()) {

			case RendererAPI::None: { AX_CORE_ASSERT(false, "None is not supported yet!"); break; }
			case RendererAPI::DirectX12: { return std::make_shared<D12TextureCube>(data, size); }

		}

		return nullptr;
	}



	Ref<Texture2D> DepthTexture::create(uint32_t width, uint32_t height) {

		switch (Renderer::getAPI()) {

			case RendererAPI::None: { AX_CORE_ASSERT(false, "None is not supported yet!"); break; }
			case RendererAPI::DirectX12: { return std::make_shared<D12DepthTexture>(width, height); }

		}

		return nullptr;
	}

}
