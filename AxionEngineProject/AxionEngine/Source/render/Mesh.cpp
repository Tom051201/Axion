#include "axpch.h"
#include "Mesh.h"

#include "AxionEngine/Source/render/Renderer.h"

#include "AxionEngine/Platform/directx/D12Mesh.h"
#include "AxionEngine/Platform/opengl/OpenGL3Mesh.h"

namespace Axion {

	Ref<Mesh> Mesh::create(const AssetHandle<Mesh>& handle, const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices) {

		switch (Renderer::getAPI()) {

			case RendererAPI::None: { AX_CORE_ASSERT(false, "None is not supported yet"); break; }
			case RendererAPI::DirectX12: { return std::make_shared<D12Mesh>(handle, vertices, indices); }
			case RendererAPI::OpenGL3: { return std::make_shared<OpenGL3Mesh>(handle, vertices, indices); }

		}
		return nullptr;

	}

}
