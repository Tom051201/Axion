#include "axpch.h"
#include "D12Buffers.h"

#include "Axion/render/GraphicsContext.h"
#include "D12Context.h"

namespace Axion {

	////////////////////////////////////////////////////////////////////////////////
	///// D12VertexBuffer //////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////////

	D12VertexBuffer::D12VertexBuffer(const std::vector<Vertex>& vertices) {
		m_vertexCount = static_cast<uint32_t>(vertices.size());
		m_size = m_stride * m_vertexCount;

		std::vector<D12Vertex> d12vertices;
		d12vertices.reserve(m_vertexCount);
		for (const auto& v : vertices) {
			d12vertices.emplace_back(v);
		}

		// Describe heap properties (upload heap)
		D3D12_HEAP_PROPERTIES heapProps = {};
		heapProps.Type = D3D12_HEAP_TYPE_UPLOAD;
		heapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		heapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
		heapProps.CreationNodeMask = 1;
		heapProps.VisibleNodeMask = 1;

		// Describe resource
		D3D12_RESOURCE_DESC bufferDesc = {};
		bufferDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		bufferDesc.Width = m_size;
		bufferDesc.Height = 1;
		bufferDesc.DepthOrArraySize = 1;
		bufferDesc.MipLevels = 1;
		bufferDesc.Format = DXGI_FORMAT_UNKNOWN;
		bufferDesc.SampleDesc.Count = 1;
		bufferDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

		// Create resource
		auto device = static_cast<D12Context*>(GraphicsContext::get()->getNativeContext())->getDevice();
		HRESULT hr = device->CreateCommittedResource(
			&heapProps,
			D3D12_HEAP_FLAG_NONE,
			&bufferDesc,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&m_buffer)
		);
		AX_THROW_IF_FAILED_HR(hr, "Failed to create D3D12 vertex buffer");

		// Upload data
		void* mappedData = nullptr;
		D3D12_RANGE readRange = { 0, 0 };
		hr = m_buffer->Map(0, &readRange, &mappedData);
		AX_THROW_IF_FAILED_HR(hr, "Failed to map vertex buffer");

		memcpy(mappedData, d12vertices.data(), m_size);
		m_buffer->Unmap(0, nullptr);

		// Fill buffer view
		m_view.BufferLocation = m_buffer->GetGPUVirtualAddress();
		m_view.StrideInBytes = m_stride;
		m_view.SizeInBytes = m_size;
	}

	D12VertexBuffer::~D12VertexBuffer() {
		
	}

	void D12VertexBuffer::bind() const {
		auto cmdList = static_cast<D12Context*>(GraphicsContext::get()->getNativeContext())->getCommandList();
		cmdList->IASetVertexBuffers(0, 1, &m_view);
		cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	}

	void D12VertexBuffer::unbind() const {

	}

	////////////////////////////////////////////////////////////////////////////////
	///// D12IndexBuffer ///////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////////

	D12IndexBuffer::D12IndexBuffer(const std::vector<uint32_t>& indices) {
		m_indexCount = static_cast<uint32_t>(indices.size());
		uint32_t bufferSize = m_indexCount * sizeof(uint32_t);

		D3D12_HEAP_PROPERTIES heapProps = {};
		heapProps.Type = D3D12_HEAP_TYPE_UPLOAD;

		D3D12_RESOURCE_DESC bufferDesc = {};
		bufferDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		bufferDesc.Alignment = 0;
		bufferDesc.Width = bufferSize;
		bufferDesc.Height = 1;
		bufferDesc.DepthOrArraySize = 1;
		bufferDesc.MipLevels = 1;
		bufferDesc.Format = DXGI_FORMAT_UNKNOWN;
		bufferDesc.SampleDesc.Count = 1;
		bufferDesc.SampleDesc.Quality = 0;
		bufferDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
		bufferDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

		auto device = static_cast<D12Context*>(GraphicsContext::get()->getNativeContext())->getDevice();
		HRESULT hr = device->CreateCommittedResource(
			&heapProps,
			D3D12_HEAP_FLAG_NONE,
			&bufferDesc,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&m_buffer)
		);
		AX_THROW_IF_FAILED_HR(hr, "Failed to create D3D12 index buffer");

		// upload data
		void* mappedData = nullptr;
		hr = m_buffer->Map(0, nullptr, &mappedData);
		AX_THROW_IF_FAILED_HR(hr, "Failed to map index buffer");

		memcpy(mappedData, indices.data(), bufferSize);
		m_buffer->Unmap(0, nullptr);

		m_view.BufferLocation = m_buffer->GetGPUVirtualAddress();
		m_view.Format = DXGI_FORMAT_R32_UINT;
		m_view.SizeInBytes = bufferSize;
	}

	D12IndexBuffer::~D12IndexBuffer() {

	}

	void D12IndexBuffer::bind() const {
		auto cmdList = static_cast<D12Context*>(GraphicsContext::get()->getNativeContext())->getCommandList();
		cmdList->IASetIndexBuffer(&m_view);
	}

	void D12IndexBuffer::unbind() const {

	}

	////////////////////////////////////////////////////////////////////////////////
	///// D12ConstantBuffer ////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////////

	D12ConstantBuffer::D12ConstantBuffer(size_t size) : m_bufferSize((size + 255) & ~255) {

		D3D12_HEAP_PROPERTIES heapProps = {};
		heapProps.Type = D3D12_HEAP_TYPE_UPLOAD;
		heapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		heapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
		heapProps.CreationNodeMask = 1;
		heapProps.VisibleNodeMask = 1;

		D3D12_RESOURCE_DESC resDesc = {};
		resDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		resDesc.Width = m_bufferSize;
		resDesc.Height = 1;
		resDesc.DepthOrArraySize = 1;
		resDesc.MipLevels = 1;
		resDesc.Format = DXGI_FORMAT_UNKNOWN;
		resDesc.SampleDesc.Count = 1;
		resDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

		auto device = static_cast<D12Context*>(GraphicsContext::get()->getNativeContext())->getDevice();
		HRESULT hr = device->CreateCommittedResource(
			&heapProps,
			D3D12_HEAP_FLAG_NONE,
			&resDesc,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&m_buffer)
		);
		AX_THROW_IF_FAILED_HR(hr, "Failed to create D3D12 constant buffer");

		D3D12_RANGE readRange = { 0, 0 };
		hr = m_buffer->Map(0, &readRange, reinterpret_cast<void**>(&m_mappedPtr));
		AX_THROW_IF_FAILED_HR(hr, "Failed to map constant buffer");
	}

	D12ConstantBuffer::~D12ConstantBuffer() {

	}

	void D12ConstantBuffer::update(const void* data, size_t size) {
		if (size > m_bufferSize) return;
		memcpy(m_mappedPtr, data, size);
	}

	void D12ConstantBuffer::bind(uint32_t slot) const {
		auto cmdList = static_cast<D12Context*>(GraphicsContext::get()->getNativeContext())->getCommandList();
		cmdList->SetGraphicsRootConstantBufferView(slot, m_buffer->GetGPUVirtualAddress());
	}

	void D12ConstantBuffer::unbind() const {

	}

}
