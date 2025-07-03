#include "axpch.h"
#include "FrameBuffer.h"

#include "AxionEngine/Source/render/Renderer.h"

#include "AxionEngine/Platform/directx/D12FrameBuffer.h"

namespace Axion {

	Ref<FrameBuffer> FrameBuffer::create(const FrameBufferSpecification& spec) {

		switch (Renderer::getAPI()) {
		
			case RendererAPI::API::None: { AX_CORE_ASSERT(false, "None is not supported yet"); }
			case RendererAPI::API::Direct3D12: { return std::make_shared<D12FrameBuffer>(spec); }

		}
		return nullptr;
	}

}
