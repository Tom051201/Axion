#include "axpch.h"
#include "D12Buffers.h"

#include "AxionEngine/Source/render/GraphicsContext.h"

#include "AxionEngine/Platform/directx/D12Context.h"

namespace Axion {

	////////////////////////////////////////////////////////////////////////////////
	///// D12VertexBuffer //////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////////

	// Static Constructor
	D12VertexBuffer::D12VertexBuffer(const std::vector<Vertex>& vertices) {
		m_type = BufferType::Static;
		m_vertexCount = static_cast<uint32_t>(vertices.size());
		m_size = m_stride * m_vertexCount;

		CD3DX12_HEAP_PROPERTIES heapProps(D3D12_HEAP_TYPE_UPLOAD);
		CD3DX12_RESOURCE_DESC bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(m_size);


		// ----- Create resource -----
		auto device = static_cast<D12Context*>(GraphicsContext::get()->getNativeContext())->getDevice();
		HRESULT hr = device->CreateCommittedResource(
			&heapProps,
			D3D12_HEAP_FLAG_NONE,
			&bufferDesc,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&m_buffer)
		);
		AX_THROW_IF_FAILED_HR(hr, "Failed to create vertex buffer");


		// ----- Upload data -----
		D3D12_RANGE readRange = { 0, 0 };
		hr = m_buffer->Map(0, &readRange, reinterpret_cast<void**>(&m_mappedPtr));
		AX_THROW_IF_FAILED_HR(hr, "Failed to map vertex buffer");

		memcpy(m_mappedPtr, vertices.data(), m_size);
		m_buffer->Unmap(0, nullptr);
		m_mappedPtr = nullptr;


		// ----- Fill buffer view -----
		m_view.BufferLocation = m_buffer->GetGPUVirtualAddress();
		m_view.StrideInBytes = m_stride;
		m_view.SizeInBytes = m_size;

		#ifdef AX_DEBUG
		m_buffer->SetName(L"VertexBuffer");
		#endif
	}

	// Dynamic Constructor
	D12VertexBuffer::D12VertexBuffer(uint32_t size, uint32_t stride) {
		m_type = BufferType::Dynamic;
		m_vertexCount = 0;
		m_size = size;
		m_stride = stride;

		CD3DX12_HEAP_PROPERTIES heapProps(D3D12_HEAP_TYPE_UPLOAD);
		CD3DX12_RESOURCE_DESC bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(m_size);


		// ----- Create resource -----
		auto device = static_cast<D12Context*>(GraphicsContext::get()->getNativeContext())->getDevice();
		HRESULT hr = device->CreateCommittedResource(
			&heapProps,
			D3D12_HEAP_FLAG_NONE,
			&bufferDesc,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&m_buffer)
		);
		AX_THROW_IF_FAILED_HR(hr, "Failed to create dynamic vertex buffer");


		// ----- Upload data -----
		D3D12_RANGE readRange = { 0, 0 };
		hr = m_buffer->Map(0, &readRange, reinterpret_cast<void**>(&m_mappedPtr));
		AX_THROW_IF_FAILED_HR(hr, "Failed to map vertex buffer");


		// ----- Fill buffer view -----
		m_view.BufferLocation = m_buffer->GetGPUVirtualAddress();
		m_view.StrideInBytes = m_stride;
		m_view.SizeInBytes = m_size;

		#ifdef AX_DEBUG
		m_buffer->SetName(L"VertexBuffer");
		#endif
	}

	D12VertexBuffer::~D12VertexBuffer() {
		release();
	}

	void D12VertexBuffer::release() {
		m_buffer.Reset();
		m_mappedPtr = nullptr;
	}

	void D12VertexBuffer::bind() const {
		auto cmdList = static_cast<D12Context*>(GraphicsContext::get()->getNativeContext())->getCommandList();
		cmdList->IASetVertexBuffers(0, 1, &m_view);
	}

	// Not required
	void D12VertexBuffer::unbind() const {}

	void D12VertexBuffer::update(const void* data, size_t size) {
		AX_CORE_ASSERT(m_mappedPtr, "Attempting to update a non-dynamic vertex buffer");
		if (size > m_size) return;

		memcpy(m_mappedPtr, data, size);
	}

	void D12VertexBuffer::update(const void* data, size_t size, size_t offset) {
		AX_CORE_ASSERT(m_mappedPtr, "Attempting to update a non-dynamic vertex buffer");
		AX_CORE_ASSERT(offset + size <= m_size, "VertexBuffer overflow");

		memcpy(static_cast<uint8_t*>(m_mappedPtr) + offset, data, size);
	}

	////////////////////////////////////////////////////////////////////////////////
	///// D12IndexBuffer ///////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////////

	// Static Constructor
	D12IndexBuffer::D12IndexBuffer(const std::vector<uint32_t>& indices) {
		m_type = BufferType::Static;
		m_indexCount = static_cast<uint32_t>(indices.size());
		uint32_t bufferSize = m_indexCount * sizeof(uint32_t);

		CD3DX12_HEAP_PROPERTIES heapProps(D3D12_HEAP_TYPE_UPLOAD);
		CD3DX12_RESOURCE_DESC bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(bufferSize);


		// ----- Create resource -----
		auto device = static_cast<D12Context*>(GraphicsContext::get()->getNativeContext())->getDevice();
		HRESULT hr = device->CreateCommittedResource(
			&heapProps,
			D3D12_HEAP_FLAG_NONE,
			&bufferDesc,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&m_buffer)
		);
		AX_THROW_IF_FAILED_HR(hr, "Failed to create index buffer");


		// ----- Upload data -----
		hr = m_buffer->Map(0, nullptr, reinterpret_cast<void**>(&m_mappedPtr));
		AX_THROW_IF_FAILED_HR(hr, "Failed to map index buffer");

		memcpy(m_mappedPtr, indices.data(), bufferSize);
		m_buffer->Unmap(0, nullptr);
		m_mappedPtr = nullptr;


		// ----- Fill buffer view -----
		m_view.BufferLocation = m_buffer->GetGPUVirtualAddress();
		m_view.Format = DXGI_FORMAT_R32_UINT;
		m_view.SizeInBytes = bufferSize;

		#ifdef AX_DEBUG
		m_buffer->SetName(L"IndexBuffer");
		#endif
	}

	// Dynamic Constructor
	D12IndexBuffer::D12IndexBuffer(uint32_t maxIndices) {
		m_type = BufferType::Dynamic;
		m_indexCount = 0;
		uint32_t bufferSize = maxIndices * sizeof(uint32_t);

		CD3DX12_HEAP_PROPERTIES heapProps(D3D12_HEAP_TYPE_UPLOAD);
		CD3DX12_RESOURCE_DESC bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(bufferSize);

		auto device = static_cast<D12Context*>(GraphicsContext::get()->getNativeContext())->getDevice();
		HRESULT hr = device->CreateCommittedResource(
			&heapProps,
			D3D12_HEAP_FLAG_NONE,
			&bufferDesc,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&m_buffer)
		);
		AX_THROW_IF_FAILED_HR(hr, "Failed to create dynamic index buffer");

		// ----- Upload data -----
		D3D12_RANGE readRange = { 0, 0 };
		hr = m_buffer->Map(0, &readRange, reinterpret_cast<void**>(&m_mappedPtr));
		AX_THROW_IF_FAILED_HR(hr, "Failed to map index buffer");

		// ----- Fill buffer view -----
		m_view.BufferLocation = m_buffer->GetGPUVirtualAddress();
		m_view.Format = DXGI_FORMAT_R32_UINT;
		m_view.SizeInBytes = bufferSize;

		#ifdef AX_DEBUG
		m_buffer->SetName(L"IndexBuffer");
		#endif
	}

	D12IndexBuffer::~D12IndexBuffer() {
		release();
	}

	void D12IndexBuffer::release() {
		m_buffer.Reset();
		m_mappedPtr = nullptr;
	}

	void D12IndexBuffer::bind() const {
		auto cmdList = static_cast<D12Context*>(GraphicsContext::get()->getNativeContext())->getCommandList();
		cmdList->IASetIndexBuffer(&m_view);
	}

	// Not required
	void D12IndexBuffer::unbind() const {}

	void D12IndexBuffer::update(const void* data, size_t size) {
		AX_CORE_ASSERT(m_mappedPtr, "Attempting to update a non-dynamic index buffer");
		AX_CORE_ASSERT(size <= m_view.SizeInBytes, "IndexBuffer overflow");

		memcpy(m_mappedPtr, data, size);
	}

	void D12IndexBuffer::update(const void* data, size_t size, size_t offset) {
		AX_CORE_ASSERT(m_mappedPtr, "Attempting to update a non-dynamic index buffer");
		AX_CORE_ASSERT(offset + size <= m_view.SizeInBytes, "IndexBuffer overflow");

		memcpy(static_cast<uint8_t*>(m_mappedPtr) + offset, data, size);
	}

	////////////////////////////////////////////////////////////////////////////////
	///// D12ConstantBuffer ////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////////

	D12ConstantBuffer::D12ConstantBuffer(size_t size) : m_bufferSize((size + 255) & ~255) {
		CD3DX12_HEAP_PROPERTIES heapProps(D3D12_HEAP_TYPE_UPLOAD);
		CD3DX12_RESOURCE_DESC resDesc = CD3DX12_RESOURCE_DESC::Buffer(m_bufferSize);


		// ----- Create resource -----
		auto device = static_cast<D12Context*>(GraphicsContext::get()->getNativeContext())->getDevice();
		HRESULT hr = device->CreateCommittedResource(
			&heapProps,
			D3D12_HEAP_FLAG_NONE,
			&resDesc,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&m_buffer)
		);
		AX_THROW_IF_FAILED_HR(hr, "Failed to create constant buffer");


		// ----- Upload data -----
		D3D12_RANGE readRange = { 0, 0 };
		hr = m_buffer->Map(0, &readRange, reinterpret_cast<void**>(&m_mappedPtr));
		AX_THROW_IF_FAILED_HR(hr, "Failed to map constant buffer");

		#ifdef AX_DEBUG
		m_buffer->SetName(L"ConstantBuffer");
		#endif
	}

	D12ConstantBuffer::~D12ConstantBuffer() {
		release();
	}
	
	void D12ConstantBuffer::release() {
		m_buffer.Reset();
		m_mappedPtr = nullptr;
	}

	void D12ConstantBuffer::update(const void* data, size_t size) {
		AX_CORE_ASSERT(size <= m_bufferSize, "ConstantBuffer overflow");
		memcpy(m_mappedPtr, data, size);
	}

	void D12ConstantBuffer::bind(uint32_t slot) const {
		auto cmdList = static_cast<D12Context*>(GraphicsContext::get()->getNativeContext())->getCommandList();
		cmdList->SetGraphicsRootConstantBufferView(slot, m_buffer->GetGPUVirtualAddress());
	}

	void D12ConstantBuffer::bind(uint32_t slot, size_t offset) const {
		AX_CORE_ASSERT(offset + 256 <= m_bufferSize, "D12 ConstantBuffer bounds check failed");
		auto cmdList = static_cast<D12Context*>(GraphicsContext::get()->getNativeContext())->getCommandList();
		cmdList->SetGraphicsRootConstantBufferView(slot, m_buffer->GetGPUVirtualAddress() + offset);
	}

	// Not required
	void D12ConstantBuffer::unbind() const {}

}
