#include "axpch.h"
#include "FrameBuffer.h"

#include "AxionEngine/Source/graphics/Renderer.h"

#include "AxionEngine/Platform/directx12/DX12FrameBuffer.h"
#include "AxionEngine/Platform/directx11/DX11FrameBuffer.h"

namespace Axion {

	Ref<FrameBuffer> FrameBuffer::create(const FrameBufferSpecification& spec) {

		switch (Renderer::getAPI()) {
		
			case RendererAPI::None: { AX_CORE_ASSERT(false, "None is not supported yet"); break; }
			case RendererAPI::DirectX12: { return MakeRef<DX12FrameBuffer>(spec); }
			case RendererAPI::DirectX11: { return MakeRef<DX11FrameBuffer>(spec); }

		}
		return nullptr;
	}

}
