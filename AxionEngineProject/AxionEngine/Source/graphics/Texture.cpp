#include "axpch.h"
#include "Texture.h"

#include "AxionEngine/Source/graphics/Renderer.h"

#include "AxionEngine/Platform/directx12/DX12Texture.h"
#include "AxionEngine/Platform/directx11/DX11Texture.h"

namespace Axion {

	Ref<Texture2D> Texture2D::create(const std::filesystem::path& path) {

		switch (Renderer::getAPI()) {

			case RendererAPI::None: { AX_CORE_ASSERT(false, "None is not supported yet!"); break; }
			case RendererAPI::DirectX12: { return MakeRef<DX12Texture2D>(path); }
			case RendererAPI::DirectX11: { return MakeRef<DX11Texture2D>(path); }

		}

		return nullptr;
	}

	Ref<Texture2D> Texture2D::create(uint32_t width, uint32_t height, void* data) {

		switch (Renderer::getAPI()) {

			case RendererAPI::None: { AX_CORE_ASSERT(false, "None is not supported yet!"); break; }
			case RendererAPI::DirectX12: { return MakeRef<DX12Texture2D>(width, height, data); }
			case RendererAPI::DirectX11: { return MakeRef<DX11Texture2D>(width, height, data); }

		}

		return nullptr;

	}

	Ref<Texture2D> Texture2D::create(const uint8_t* data, size_t size) {

		switch (Renderer::getAPI()) {

			case RendererAPI::None: { AX_CORE_ASSERT(false, "None is not supported yet!"); break; }
			case RendererAPI::DirectX12: { return MakeRef<DX12Texture2D>(data, size); }
			case RendererAPI::DirectX11: { return MakeRef<DX11Texture2D>(data, size); }

		}

		return nullptr;

	}



	Ref<TextureCube> TextureCube::create(const std::array<std::filesystem::path, 6>& paths) {

		switch (Renderer::getAPI()) {

			case RendererAPI::None: { AX_CORE_ASSERT(false, "None is not supported yet!"); break; }
			case RendererAPI::DirectX12: { return MakeRef<DX12TextureCube>(paths); }
			case RendererAPI::DirectX11: { return MakeRef<DX11TextureCube>(paths); }

		}

		return nullptr;
	}

	Ref<TextureCube> TextureCube::create(const std::filesystem::path& filePath) {

		switch (Renderer::getAPI()) {

			case RendererAPI::None: { AX_CORE_ASSERT(false, "None is not supported yet!"); break; }
			case RendererAPI::DirectX12: { return MakeRef<DX12TextureCube>(filePath); }
			case RendererAPI::DirectX11: { return MakeRef<DX11TextureCube>(filePath); }

		}

		return nullptr;
	}

	Ref<TextureCube> TextureCube::create(const uint8_t* data, size_t size) {

		switch (Renderer::getAPI()) {

			case RendererAPI::None: { AX_CORE_ASSERT(false, "None is not supported yet!"); break; }
			case RendererAPI::DirectX12: { return MakeRef<DX12TextureCube>(data, size); }
			case RendererAPI::DirectX11: { return MakeRef<DX11TextureCube>(data, size); }

		}

		return nullptr;
	}



	Ref<Texture2D> DepthTexture::create(uint32_t width, uint32_t height) {

		switch (Renderer::getAPI()) {

			case RendererAPI::None: { AX_CORE_ASSERT(false, "None is not supported yet!"); break; }
			case RendererAPI::DirectX12: { return MakeRef<DX12DepthTexture>(width, height); }
			case RendererAPI::DirectX11: { return MakeRef<DX11DepthTexture>(width, height); }

		}

		return nullptr;
	}

}
