#include "axpch.h"
#include "Buffers.h"

#include "AxionEngine/Source/render/Renderer.h"

#include "AxionEngine/Platform/directx/D12Buffers.h"
#include "AxionEngine/Platform/opengl/OpenglBuffers.h"

namespace Axion {

	////////////////////////////////////////////////////////////////////////////////
	///// BufferLayout /////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////////

	void BufferLayout::calculateOffsetAndStride() {
		uint32_t offset = 0;
		m_stride = 0;

		for (auto& element : m_elements) {
			element.offset = offset;
			offset += element.size;
			m_stride += element.size;
		}
	}

	////////////////////////////////////////////////////////////////////////////////
	///// VertexBuffer /////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////////

	Ref<VertexBuffer> VertexBuffer::create(const std::vector<Vertex>& vertices) {

		switch (Renderer::getAPI()) {
			
			case RendererAPI::None: { AX_ASSERT(false, "None is not supported yet"); return nullptr; }
			case RendererAPI::Direct3D12: { return std::make_shared<D12VertexBuffer>(vertices); }
			case RendererAPI::OpenGL: { return std::make_shared<OpenglVertexBuffer>(vertices); }

		}
		return nullptr;
	}

	////////////////////////////////////////////////////////////////////////////////
	///// IndexBuffer //////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////////

	Ref<IndexBuffer> IndexBuffer::create(const std::vector<uint32_t>& indices) {
	
		switch (Renderer::getAPI()) {
			
			case RendererAPI::None: { AX_ASSERT(false, "None is not supported yet"); return nullptr; }
			case RendererAPI::Direct3D12: { return std::make_shared<D12IndexBuffer>(indices); }
			case RendererAPI::OpenGL: { return std::make_shared<OpenglIndexBuffer>(indices); }
			
		}
		return nullptr;

	}

	////////////////////////////////////////////////////////////////////////////////
	///// ConstatBuffer ////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////////

	Ref<ConstantBuffer> ConstantBuffer::create(size_t size) {
		switch (Renderer::getAPI()) {
			
			case RendererAPI::None: { AX_ASSERT(false, "None is not supported yet"); return nullptr; }
			case RendererAPI::Direct3D12: { return std::make_shared<D12ConstantBuffer>(size); }
			case RendererAPI::OpenGL: { return std::make_shared<OpenglConstantBuffer>(size); }

		}
		return nullptr;
	}

}
