#include "axpch.h"
#include "Texture.h"

#include "AxionEngine/Source/render/Renderer.h"

#include "AxionEngine/Platform/directx/D12Texture.h"
#include "AxionEngine/Platform/opengl/OpenGL3Texture.h"

namespace Axion {

	Ref<Texture2D> Texture2D::create(const std::string& path) {

		switch (Renderer::getAPI()) {
		
			case RendererAPI::None: { AX_CORE_ASSERT(false, "None is not supported yet!"); break; }
			case RendererAPI::DirectX12: { return std::make_shared<D12Texture2D>(path); }
			case RendererAPI::OpenGL3: { return std::make_shared<OpenGL3Texture2D>(path); }

		}

		return nullptr;
	}



	Ref<TextureCube> TextureCube::create(const std::array<std::string, 6>& paths) {
		
		switch (Renderer::getAPI()) {

			case RendererAPI::None: { AX_CORE_ASSERT(false, "None is not supported yet!"); break; }
			case RendererAPI::DirectX12: { return std::make_shared<D12TextureCube>(paths); }
			case RendererAPI::OpenGL3: { return std::make_shared<OpenGL3TextureCube>(paths); }

		}

		return nullptr;
	}

}
