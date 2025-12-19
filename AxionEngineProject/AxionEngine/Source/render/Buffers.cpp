#include "axpch.h"
#include "Buffers.h"

#include "AxionEngine/Source/render/Renderer.h"

#include "AxionEngine/Platform/directx/D12Buffers.h"
#include "AxionEngine/Platform/opengl/OpenGL3Buffers.h"

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
			
			case RendererAPI::None: { AX_CORE_ASSERT(false, "None is not supported yet"); return nullptr; }
			case RendererAPI::DirectX12: { return std::make_shared<D12VertexBuffer>(vertices); }
			case RendererAPI::OpenGL3: { return std::make_shared<OpenGL3VertexBuffer>(vertices); }

		}
		return nullptr;
	}

	Ref<VertexBuffer> VertexBuffer::createDynamic(uint32_t size, uint32_t stride) {
		switch (Renderer::getAPI()) {

			case RendererAPI::None: { AX_CORE_ASSERT(false, "None is not supported yet"); return nullptr; }
			case RendererAPI::DirectX12: { return std::make_shared<D12VertexBuffer>(size, stride); }
			case RendererAPI::OpenGL3: { AX_CORE_ASSERT(false, "Opengl is not supported yet!"); /*TODO: Add functionality*/ }

		}
		return nullptr;
	}

	////////////////////////////////////////////////////////////////////////////////
	///// IndexBuffer //////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////////

	Ref<IndexBuffer> IndexBuffer::create(const std::vector<uint32_t>& indices) {
	
		switch (Renderer::getAPI()) {
			
			case RendererAPI::None: { AX_CORE_ASSERT(false, "None is not supported yet"); return nullptr; }
			case RendererAPI::DirectX12: { return std::make_shared<D12IndexBuffer>(indices); }
			case RendererAPI::OpenGL3: { return std::make_shared<OpenGL3IndexBuffer>(indices); }
			
		}
		return nullptr;

	}

	Ref<IndexBuffer> IndexBuffer::createDynamic(uint32_t maxIndices) {

		switch (Renderer::getAPI()) {

			case RendererAPI::None: { AX_CORE_ASSERT(false, "None is not supported yet"); return nullptr; }
			case RendererAPI::DirectX12: { return std::make_shared<D12IndexBuffer>(maxIndices); }
			case RendererAPI::OpenGL3: { return std::make_shared<OpenGL3IndexBuffer>(maxIndices); }

		}
		return nullptr;
	}

	////////////////////////////////////////////////////////////////////////////////
	///// ConstatBuffer ////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////////

	Ref<ConstantBuffer> ConstantBuffer::create(size_t size) {
		switch (Renderer::getAPI()) {
			
			case RendererAPI::None: { AX_CORE_ASSERT(false, "None is not supported yet"); return nullptr; }
			case RendererAPI::DirectX12: { return std::make_shared<D12ConstantBuffer>(size); }
			case RendererAPI::OpenGL3: { return std::make_shared<OpenGL3ConstantBuffer>(size); }

		}
		return nullptr;
	}

}
