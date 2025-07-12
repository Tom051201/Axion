#include "axpch.h"
#include "FrameBuffer.h"

#include "AxionEngine/Source/render/Renderer.h"

#include "AxionEngine/Platform/directx/D12FrameBuffer.h"
#include "AxionEngine/Platform/opengl/OpenGL3FrameBuffer.h"

namespace Axion {

	Ref<FrameBuffer> FrameBuffer::create(const FrameBufferSpecification& spec) {

		switch (Renderer::getAPI()) {
		
			case RendererAPI::None: { AX_CORE_ASSERT(false, "None is not supported yet"); break; }
			case RendererAPI::DirectX12: { return std::make_shared<D12FrameBuffer>(spec); }
			case RendererAPI::OpenGL3: { return std::make_shared<OpenGL3FrameBuffer>(spec); }

		}
		return nullptr;
	}

}
