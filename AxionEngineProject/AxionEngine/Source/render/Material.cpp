#include "axpch.h"
#include "Material.h"

#include "AxionEngine/Source/render/Renderer.h"

#include "AxionEngine/Platform/directx/D12Material.h"
#include "AxionEngine/Platform/opengl/OpenGL3Material.h"

namespace Axion {

	Ref<Material> Material::create(const std::string& name, const Vec4& color, const Ref<Shader>& shader) {

		switch (Renderer::getAPI()) {

			case RendererAPI::None: { AX_ASSERT(false, "None is not supported yet"); break; }
			case RendererAPI::DirectX12: { return std::make_shared<D12Material>(name, color, shader); }
			case RendererAPI::OpenGL3: { return std::make_shared<OpenGL3Material>(name, color, shader); }

		}
		return nullptr;

	}

}
