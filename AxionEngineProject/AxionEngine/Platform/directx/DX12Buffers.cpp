#include "axpch.h"
#include "DX12Buffers.h"

#include "AxionEngine/Source/render/GraphicsContext.h"

#include "AxionEngine/Platform/directx/DX12Context.h"

namespace Axion {

	////////////////////////////////////////////////////////////////////////////////
	///// DX12VertexBuffer /////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////////

	// Static Constructor
	DX12VertexBuffer::DX12VertexBuffer(const void* data, uint32_t size, uint32_t stride) {
		m_type = BufferType::Static;
		m_stride = stride;
		m_size = size;
		m_vertexCount = m_size / m_stride;

		CD3DX12_HEAP_PROPERTIES heapProps(D3D12_HEAP_TYPE_UPLOAD);
		CD3DX12_RESOURCE_DESC bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(m_size);


		// ----- Create resource -----
		auto device = static_cast<DX12Context*>(GraphicsContext::get()->getNativeContext())->getDevice();
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

		memcpy(m_mappedPtr, data, m_size);
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
	DX12VertexBuffer::DX12VertexBuffer(uint32_t size, uint32_t stride) {
		m_type = BufferType::Dynamic;
		m_vertexCount = 0;
		m_size = size;
		m_stride = stride;

		CD3DX12_HEAP_PROPERTIES heapProps(D3D12_HEAP_TYPE_UPLOAD);
		CD3DX12_RESOURCE_DESC bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(m_size);


		// ----- Create resource -----
		auto device = static_cast<DX12Context*>(GraphicsContext::get()->getNativeContext())->getDevice();
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

	DX12VertexBuffer::~DX12VertexBuffer() {
		release();
	}

	void DX12VertexBuffer::release() {
		m_buffer.Reset();
		m_mappedPtr = nullptr;
	}

	void DX12VertexBuffer::bind(uint32_t slot, uint32_t offset) const {
		auto cmdList = static_cast<DX12Context*>(GraphicsContext::get()->getNativeContext())->getCommandList();
		D3D12_VERTEX_BUFFER_VIEW view = m_view;
		view.BufferLocation += offset;
		view.SizeInBytes -= offset;
		cmdList->IASetVertexBuffers(slot, 1, &view);
	}

	// Not required
	void DX12VertexBuffer::unbind() const {}

	void DX12VertexBuffer::update(const void* data, size_t size) {
		AX_CORE_ASSERT(m_mappedPtr, "Attempting to update a non-dynamic vertex buffer");
		if (size > m_size) return;

		memcpy(m_mappedPtr, data, size);
	}

	void DX12VertexBuffer::update(const void* data, size_t size, size_t offset) {
		AX_CORE_ASSERT(m_mappedPtr, "Attempting to update a non-dynamic vertex buffer");
		AX_CORE_ASSERT(offset + size <= m_size, "VertexBuffer overflow");

		memcpy(static_cast<uint8_t*>(m_mappedPtr) + offset, data, size);
	}

	uint32_t DX12VertexBuffer::append(const void* data, size_t size) {
		uint32_t writeOffset = m_currentOffset;
		memcpy(m_mappedPtr + writeOffset, data, size);
		m_currentOffset += (uint32_t)size;
		return writeOffset;
	}

	void DX12VertexBuffer::resetOffset() {
		m_currentOffset = 0;
	}

	////////////////////////////////////////////////////////////////////////////////
	///// DX12IndexBuffer //////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////////

	// Static Constructor
	DX12IndexBuffer::DX12IndexBuffer(const std::vector<uint32_t>& indices) {
		m_type = BufferType::Static;
		m_indexCount = static_cast<uint32_t>(indices.size());
		uint32_t bufferSize = m_indexCount * sizeof(uint32_t);

		CD3DX12_HEAP_PROPERTIES heapProps(D3D12_HEAP_TYPE_UPLOAD);
		CD3DX12_RESOURCE_DESC bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(bufferSize);


		// ----- Create resource -----
		auto device = static_cast<DX12Context*>(GraphicsContext::get()->getNativeContext())->getDevice();
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
	DX12IndexBuffer::DX12IndexBuffer(uint32_t maxIndices) {
		m_type = BufferType::Dynamic;
		m_indexCount = 0;
		uint32_t bufferSize = maxIndices * sizeof(uint32_t);

		CD3DX12_HEAP_PROPERTIES heapProps(D3D12_HEAP_TYPE_UPLOAD);
		CD3DX12_RESOURCE_DESC bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(bufferSize);

		auto device = static_cast<DX12Context*>(GraphicsContext::get()->getNativeContext())->getDevice();
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

	DX12IndexBuffer::~DX12IndexBuffer() {
		release();
	}

	void DX12IndexBuffer::release() {
		m_buffer.Reset();
		m_mappedPtr = nullptr;
	}

	void DX12IndexBuffer::bind() const {
		auto cmdList = static_cast<DX12Context*>(GraphicsContext::get()->getNativeContext())->getCommandList();
		cmdList->IASetIndexBuffer(&m_view);
	}

	// Not required
	void DX12IndexBuffer::unbind() const {}

	void DX12IndexBuffer::update(const void* data, size_t size) {
		AX_CORE_ASSERT(m_mappedPtr, "Attempting to update a non-dynamic index buffer");
		AX_CORE_ASSERT(size <= m_view.SizeInBytes, "IndexBuffer overflow");

		memcpy(m_mappedPtr, data, size);
	}

	void DX12IndexBuffer::update(const void* data, size_t size, size_t offset) {
		AX_CORE_ASSERT(m_mappedPtr, "Attempting to update a non-dynamic index buffer");
		AX_CORE_ASSERT(offset + size <= m_view.SizeInBytes, "IndexBuffer overflow");

		memcpy(static_cast<uint8_t*>(m_mappedPtr) + offset, data, size);
	}

	////////////////////////////////////////////////////////////////////////////////
	///// DX12ConstantBuffer ///////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////////

	DX12ConstantBuffer::DX12ConstantBuffer(size_t size) : m_bufferSize((size + 255) & ~255) {
		CD3DX12_HEAP_PROPERTIES heapProps(D3D12_HEAP_TYPE_UPLOAD);
		CD3DX12_RESOURCE_DESC resDesc = CD3DX12_RESOURCE_DESC::Buffer(m_bufferSize);


		// ----- Create resource -----
		auto device = static_cast<DX12Context*>(GraphicsContext::get()->getNativeContext())->getDevice();
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

	DX12ConstantBuffer::~DX12ConstantBuffer() {
		release();
	}
	
	void DX12ConstantBuffer::release() {
		m_buffer.Reset();
		m_mappedPtr = nullptr;
	}

	void DX12ConstantBuffer::bind(uint32_t slot) const {
		auto cmdList = static_cast<DX12Context*>(GraphicsContext::get()->getNativeContext())->getCommandList();
		cmdList->SetGraphicsRootConstantBufferView(slot, m_buffer->GetGPUVirtualAddress());
	}

	void DX12ConstantBuffer::bind(uint32_t slot, size_t offset) const {
		AX_CORE_ASSERT(offset + 256 <= m_bufferSize, "DirectX12 ConstantBuffer bounds check failed");
		auto cmdList = static_cast<DX12Context*>(GraphicsContext::get()->getNativeContext())->getCommandList();
		cmdList->SetGraphicsRootConstantBufferView(slot, m_buffer->GetGPUVirtualAddress() + offset);
	}

	// Not required
	void DX12ConstantBuffer::unbind() const {}

	void DX12ConstantBuffer::update(const void* data, size_t size) {
		AX_CORE_ASSERT(size <= m_bufferSize, "ConstantBuffer overflow");
		memcpy(m_mappedPtr, data, size);
	}

	uint32_t DX12ConstantBuffer::append(const void* data, size_t size) {
		uint32_t alignedSize = (size + 255) & ~255;

		AX_CORE_ASSERT(m_currentOffset + alignedSize <= m_bufferSize, "ConstantBuffer overflow");

		uint32_t writeOffset = m_currentOffset;
		memcpy(m_mappedPtr + writeOffset, data, size);
		m_currentOffset += alignedSize;

		return writeOffset;
	}

	void DX12ConstantBuffer::resetOffset() {
		m_currentOffset = 0;
	}

	////////////////////////////////////////////////////////////////////////////////
	///// DX12StructuredBuffer /////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////////

	DX12StructuredBuffer::DX12StructuredBuffer(uint32_t elementSize, uint32_t elementCount)
		: m_elementSize(elementSize), m_elementCount(elementCount) {

		m_bufferSize = m_elementSize * m_elementCount;

		CD3DX12_HEAP_PROPERTIES heapProps(D3D12_HEAP_TYPE_UPLOAD);
		CD3DX12_RESOURCE_DESC resDesc = CD3DX12_RESOURCE_DESC::Buffer(m_bufferSize);

		// ----- Create Resource -----
		auto device = static_cast<DX12Context*>(GraphicsContext::get()->getNativeContext())->getDevice();
		HRESULT hr = device->CreateCommittedResource(
			&heapProps,
			D3D12_HEAP_FLAG_NONE,
			&resDesc,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&m_buffer)
		);
		AX_THROW_IF_FAILED_HR(hr, "Failed to create structured buffer");

		// ----- Upload data -----
		D3D12_RANGE readRange = { 0, 0 };
		hr = m_buffer->Map(0, &readRange, reinterpret_cast<void**>(&m_mappedPtr));
		AX_THROW_IF_FAILED_HR(hr, "Failed to map structured buffer");

		#ifdef AX_DEBUG
		m_buffer->SetName(L"StructuredBuffer");
		#endif
	}

	DX12StructuredBuffer::~DX12StructuredBuffer() {
		release();
	}

	void DX12StructuredBuffer::release() {
		if (m_buffer && m_mappedPtr) {
			m_buffer->Unmap(0, nullptr);
		}
		m_buffer.Reset();
		m_mappedPtr = nullptr;
	}

	void DX12StructuredBuffer::bind(uint32_t slot) const {
		auto cmdList = static_cast<DX12Context*>(GraphicsContext::get()->getNativeContext())->getCommandList();
		cmdList->SetGraphicsRootShaderResourceView(slot, m_buffer->GetGPUVirtualAddress());
	}

	void DX12StructuredBuffer::bind(uint32_t slot, size_t offset) const {
		auto cmdList = static_cast<DX12Context*>(GraphicsContext::get()->getNativeContext())->getCommandList();
		cmdList->SetGraphicsRootShaderResourceView(slot, m_buffer->GetGPUVirtualAddress() + offset);
	}

	void DX12StructuredBuffer::unbind() const {}

	void DX12StructuredBuffer::update(const void* data, size_t size) {
		AX_CORE_ASSERT(size <= m_bufferSize, "StructuredBuffer overflow");
		memcpy(m_mappedPtr, data, size);
	}

	void DX12StructuredBuffer::update(const void* data, size_t size, size_t offset) {
		AX_CORE_ASSERT(offset + size <= m_bufferSize, "StructuredBuffer overflow");
		memcpy(m_mappedPtr + offset, data, size);
	}

	uint32_t DX12StructuredBuffer::append(const void* data, size_t size) {
		AX_CORE_ASSERT(m_currentOffset + size <= m_bufferSize, "StructuredBuffer overflow");
		uint32_t writeOffset = m_currentOffset;
		memcpy(m_mappedPtr + writeOffset, data, size);
		m_currentOffset += (uint32_t)size;
		return writeOffset;
	}

	void DX12StructuredBuffer::resetOffset() {
		m_currentOffset = 0;
	}

}
