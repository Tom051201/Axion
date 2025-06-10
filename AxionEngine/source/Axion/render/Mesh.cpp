#include "axpch.h"
#include "Mesh.h"

#include "Axion/render/Renderer.h"

#include "platform/directx/D12Mesh.h"

namespace Axion {

	Ref<Mesh> Mesh::create(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices) {

		switch (Renderer::getAPI()) {

			case RendererAPI::API::None: { AX_ASSERT(false, "None is not supported yet"); }
			case RendererAPI::API::Direct3D12: { return std::make_shared<D12Mesh>(vertices, indices); }

		}
		return nullptr;

	}

}
