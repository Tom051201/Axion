#include "axpch.h"
#include "D12Buffers.h"

#include "AxionEngine/Source/render/GraphicsContext.h"

#include "AxionEngine/Platform/directx/D12Context.h"

namespace Axion {

	////////////////////////////////////////////////////////////////////////////////
	///// D12VertexBuffer //////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////////

	D12VertexBuffer::D12VertexBuffer(const std::vector<Vertex>& vertices) {
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
		void* mappedData = nullptr;
		D3D12_RANGE readRange = { 0, 0 };
		hr = m_buffer->Map(0, &readRange, &mappedData);
		AX_THROW_IF_FAILED_HR(hr, "Failed to map vertex buffer");

		memcpy(mappedData, vertices.data(), m_size);
		m_buffer->Unmap(0, nullptr);


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
	}

	void D12VertexBuffer::bind() const {
		auto cmdList = static_cast<D12Context*>(GraphicsContext::get()->getNativeContext())->getCommandList();
		cmdList->IASetVertexBuffers(0, 1, &m_view);
		cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	}

	// Not required
	void D12VertexBuffer::unbind() const {}

	////////////////////////////////////////////////////////////////////////////////
	///// D12IndexBuffer ///////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////////

	D12IndexBuffer::D12IndexBuffer(const std::vector<uint32_t>& indices) {
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
		void* mappedData = nullptr;
		hr = m_buffer->Map(0, nullptr, &mappedData);
		AX_THROW_IF_FAILED_HR(hr, "Failed to map index buffer");

		memcpy(mappedData, indices.data(), bufferSize);
		m_buffer->Unmap(0, nullptr);


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
	}

	void D12IndexBuffer::bind() const {
		auto cmdList = static_cast<D12Context*>(GraphicsContext::get()->getNativeContext())->getCommandList();
		cmdList->IASetIndexBuffer(&m_view);
	}

	// Not required
	void D12IndexBuffer::unbind() const {}

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
	}

	void D12ConstantBuffer::update(const void* data, size_t size) {
		if (size > m_bufferSize) return;
		memcpy(m_mappedPtr, data, size);
	}

	void D12ConstantBuffer::bind(uint32_t slot) const {
		auto cmdList = static_cast<D12Context*>(GraphicsContext::get()->getNativeContext())->getCommandList();
		cmdList->SetGraphicsRootConstantBufferView(slot, m_buffer->GetGPUVirtualAddress());
	}

	// Not required
	void D12ConstantBuffer::unbind() const {}

}
