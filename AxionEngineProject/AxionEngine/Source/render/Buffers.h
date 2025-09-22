#pragma once
#include "axpch.h"

#include "AxionEngine/Source/core/Core.h"
#include "AxionEngine/Source/render/Vertex.h"

namespace Axion {

	////////////////////////////////////////////////////////////////////////////////
	///// BufferElement ////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////////

	enum class ShaderDataType {
		None = 0,
		Float, Float2, Float3, Float4,
		Int, Int2, Int3, Int4,
		Bool
	};

	static uint32_t ShaderDataTypeSize(ShaderDataType type) {
		switch (type) {
			case Axion::ShaderDataType::None:		return 0;
			case Axion::ShaderDataType::Float:		return sizeof(float);
			case Axion::ShaderDataType::Float2:		return sizeof(float) * 2;
			case Axion::ShaderDataType::Float3:		return sizeof(float) * 3;
			case Axion::ShaderDataType::Float4:		return sizeof(float) * 4;
			case Axion::ShaderDataType::Int:		return sizeof(int);
			case Axion::ShaderDataType::Int2:		return sizeof(int) * 2;
			case Axion::ShaderDataType::Int3:		return sizeof(int) * 3;
			case Axion::ShaderDataType::Int4:		return sizeof(int) * 4;
			case Axion::ShaderDataType::Bool:		return sizeof(bool);
			default: { AX_CORE_ASSERT(false, "Unkown ShaderDataType"); return 0; }
		}
	}

	struct BufferElement {
		std::string name;
		ShaderDataType type;
		uint32_t size;
		uint32_t offset;

		BufferElement(const std::string& name, ShaderDataType type) : name(name), type(type), size(ShaderDataTypeSize(type)), offset(0) {}
	};

	////////////////////////////////////////////////////////////////////////////////
	///// BufferLayout /////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////////

	class BufferLayout {
	public:

		BufferLayout() {}

		BufferLayout(const std::initializer_list<BufferElement>& elements)
			: m_elements(elements) {
			calculateOffsetAndStride();
		}

		BufferLayout(const std::vector<BufferElement>& elements)
			: m_elements(elements) {
			calculateOffsetAndStride();
		}

		void calculateOffsetAndStride();

		const std::vector<BufferElement>& getElements() const { return m_elements; }
		std::vector<BufferElement>& getElements() { return m_elements; }

	private:

		std::vector<BufferElement> m_elements;
		uint32_t m_stride = 0;

	};

	////////////////////////////////////////////////////////////////////////////////
	///// VertexBuffer /////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////////

	class VertexBuffer {
	public:

		virtual ~VertexBuffer() = default;

		virtual void release() = 0;

		virtual void bind() const = 0;
		virtual void unbind() const = 0;

		virtual void setLayout(const BufferLayout& layout) = 0;
		virtual const BufferLayout& getLayout() const = 0;

		virtual uint32_t getVertexCount() const = 0; // in vertices
		virtual uint32_t getSize() const = 0; // in bytes

		static Ref<VertexBuffer> create(const std::vector<Vertex>& vertices);

	};

	////////////////////////////////////////////////////////////////////////////////
	///// IndexBuffer //////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////////

	class IndexBuffer {
	public:

		virtual ~IndexBuffer() = default;

		virtual void release() = 0;

		virtual void bind() const = 0;
		virtual void unbind() const = 0;

		virtual uint32_t getIndexCount() const = 0;

		static Ref<IndexBuffer> create(const std::vector<uint32_t>& indices);
	};

	////////////////////////////////////////////////////////////////////////////////
	///// ConstatBuffer ////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////////

	class ConstantBuffer {
	public:

		virtual ~ConstantBuffer() = default;

		virtual void release() = 0;

		virtual void bind(uint32_t slot) const = 0;
		virtual void unbind() const = 0;

		virtual void update(const void* data, size_t size) = 0;

		virtual uint32_t getSize() const = 0;

		static Ref<ConstantBuffer> create(size_t size);

	};



	struct alignas(16) ObjectBuffer {
		DirectX::XMFLOAT4 color;
		DirectX::XMMATRIX modelMatrix;
	};

}
