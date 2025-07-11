#include "axpch.h"
#include "Texture.h"

#include "AxionEngine/Source/render/Renderer.h"

#include "AxionEngine/Platform/directx/D12Texture.h"

namespace Axion {

	Ref<Texture2D> Texture2D::create(const std::string& path) {

		switch (Renderer::getAPI()) {
		
			case RendererAPI::None: { AX_CORE_ASSERT(false, "None is not supported yet!"); }
			case RendererAPI::Direct3D12: { return std::make_shared<D12Texture2D>(path); }

		}

		return nullptr;
	}

}
