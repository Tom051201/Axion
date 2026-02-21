#include "axpch.h"
#include "Pipeline.h"

#include "AxionEngine/Source/render/Renderer.h"

#include "AxionEngine/Platform/directx/D12Pipeline.h"

namespace Axion {

	Ref<Pipeline> Pipeline::create(const PipelineSpecification& spec) {

		switch (Renderer::getAPI()) {

			case RendererAPI::None: { AX_CORE_ASSERT(false, "None is not supported yet"); return nullptr; }
			case RendererAPI::DirectX12: { return std::make_shared<D12Pipeline>(spec); }
			case RendererAPI::OpenGL3: { /*return std::make_shared<OpenGL3Pipeline>(spec); TODO*/ }

		}
		return nullptr;

	}

}
