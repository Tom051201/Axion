#include "axpch.h"
#include "Texture.h"

#include "Axion/render/Renderer.h"

namespace Axion {

	Ref<Texture2D> Texture2D::create(const std::string& path) {
		
		switch (Renderer::getAPI()) {

			case RendererAPI::API::None: { AX_ASSERT(false, "None is not supported yet"); }
			//case RendererAPI::API::Direct3D12: { return std::make_shared<D12Texture2D>(path); }

		}
		return nullptr;

	}


}
