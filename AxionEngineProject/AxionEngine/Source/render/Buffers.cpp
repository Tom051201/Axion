#include "axpch.h"
#include "Buffers.h"

#include "AxionEngine/Source/render/Renderer.h"

#include "AxionEngine/Platform/directx/DX12Buffers.h"

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

	Ref<VertexBuffer> VertexBuffer::create(const void* data, uint32_t size, uint32_t stride) {

		switch (Renderer::getAPI()) {
			
			case RendererAPI::None: { AX_CORE_ASSERT(false, "None is not supported yet"); return nullptr; }
			case RendererAPI::DirectX12: { return std::make_shared<DX12VertexBuffer>(data, size, stride); }

		}
		return nullptr;
	}

	Ref<VertexBuffer> VertexBuffer::createDynamic(uint32_t size, uint32_t stride) {
		switch (Renderer::getAPI()) {

			case RendererAPI::None: { AX_CORE_ASSERT(false, "None is not supported yet"); return nullptr; }
			case RendererAPI::DirectX12: { return std::make_shared<DX12VertexBuffer>(size, stride); }

		}
		return nullptr;
	}

	////////////////////////////////////////////////////////////////////////////////
	///// IndexBuffer //////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////////

	Ref<IndexBuffer> IndexBuffer::create(const std::vector<uint32_t>& indices) {
	
		switch (Renderer::getAPI()) {
			
			case RendererAPI::None: { AX_CORE_ASSERT(false, "None is not supported yet"); return nullptr; }
			case RendererAPI::DirectX12: { return std::make_shared<DX12IndexBuffer>(indices); }
			
		}
		return nullptr;

	}

	Ref<IndexBuffer> IndexBuffer::createDynamic(uint32_t maxIndices) {

		switch (Renderer::getAPI()) {

			case RendererAPI::None: { AX_CORE_ASSERT(false, "None is not supported yet"); return nullptr; }
			case RendererAPI::DirectX12: { return std::make_shared<DX12IndexBuffer>(maxIndices); }

		}
		return nullptr;
	}

	////////////////////////////////////////////////////////////////////////////////
	///// ConstatBuffer ////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////////

	Ref<ConstantBuffer> ConstantBuffer::create(size_t size) {
		switch (Renderer::getAPI()) {
			
			case RendererAPI::None: { AX_CORE_ASSERT(false, "None is not supported yet"); return nullptr; }
			case RendererAPI::DirectX12: { return std::make_shared<DX12ConstantBuffer>(size); }

		}
		return nullptr;
	}

	////////////////////////////////////////////////////////////////////////////////
	///// StructuredBuffer /////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////////

	Ref<StructuredBuffer> StructuredBuffer::create(uint32_t elementSize, uint32_t elementCount) {
		switch (Renderer::getAPI()) {

			case RendererAPI::None: { AX_CORE_ASSERT(false, "None is not supported yet"); return nullptr; }
			case RendererAPI::DirectX12: { return std::make_shared<DX12StructuredBuffer>(elementSize, elementCount); }

		}
		return nullptr;
	}

}
