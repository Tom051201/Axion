#include "axpch.h"
#include "DX12Buffers.h"

#include "AxionEngine/Source/graphics/GraphicsContext.h"
#include "AxionEngine/Source/graphics/RenderContext.h"

#include "AxionEngine/Platform/directx12/DX12Context.h"

namespace Axion {

	constexpr uint32_t MAX_FRAMES_IN_FLIGHT = 3;

	////////////////////////////////////////////////////////////////////////////////
	///// DX12VertexBuffer /////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////////

	// -- Static Constructor --
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
			&heapProps, D3D12_HEAP_FLAG_NONE, &bufferDesc,
			D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&m_buffer)
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
		m_buffer->SetName(L"DX12 Static VertexBuffer");
		#endif
	}

	// -- Dynamic Constructor --
	DX12VertexBuffer::DX12VertexBuffer(uint32_t size, uint32_t stride) {
		m_type = BufferType::Dynamic;
		m_vertexCount = 0;
		m_perFrameSize = size;
		m_stride = stride;
		m_size = m_perFrameSize * MAX_FRAMES_IN_FLIGHT;

		CD3DX12_HEAP_PROPERTIES heapProps(D3D12_HEAP_TYPE_UPLOAD);
		CD3DX12_RESOURCE_DESC bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(m_size);

		// ----- Create resource -----
		auto device = static_cast<DX12Context*>(GraphicsContext::get()->getNativeContext())->getDevice();
		HRESULT hr = device->CreateCommittedResource(
			&heapProps, D3D12_HEAP_FLAG_NONE, &bufferDesc,
			D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&m_buffer)
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
		m_buffer->SetName(L"DX12 Dynamic VertexBuffer");
		#endif
	}

	DX12VertexBuffer::~DX12VertexBuffer() {
		release();
	}

	void DX12VertexBuffer::release() {
		m_buffer.Reset();
		m_mappedPtr = nullptr;
	}

	void DX12VertexBuffer::bind(RenderContext* renderContext, uint32_t slot, uint32_t offset) const {
		auto cmdList = static_cast<DX12RenderContext*>(renderContext)->getCmdList();
		D3D12_VERTEX_BUFFER_VIEW view = m_view;
		view.BufferLocation += offset;
		view.SizeInBytes -= offset;
		cmdList->IASetVertexBuffers(slot, 1, &view);
	}

	void DX12VertexBuffer::unbind() const {}

	void DX12VertexBuffer::update(const void* data, size_t size) {
		AX_CORE_ASSERT(m_mappedPtr, "Attempting to update a non-dynamic vertex buffer");
		if (size > m_perFrameSize) return;
		for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
			memcpy(m_mappedPtr + (i * m_perFrameSize), data, size);
		}
	}

	void DX12VertexBuffer::update(const void* data, size_t size, size_t offset) {
		AX_CORE_ASSERT(m_mappedPtr, "Attempting to update a non-dynamic vertex buffer");
		AX_CORE_ASSERT(offset + size <= m_size, "VertexBuffer overflow");
		for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
			memcpy(m_mappedPtr + (i * m_perFrameSize) + offset, data, size);
		}
	}

	uint32_t DX12VertexBuffer::append(const void* data, size_t size) {
		std::lock_guard<std::mutex> lock(m_appendMutex);

		auto* context = static_cast<DX12Context*>(GraphicsContext::get()->getNativeContext());
		uint32_t currentFrame = context->getCurrentFrameIndex();
		uint32_t frameStart = currentFrame * m_perFrameSize;
		uint32_t currentFrameBytesWritten = m_currentOffset - frameStart;

		if (currentFrameBytesWritten + size > m_perFrameSize) {
			uint32_t newPerFrameSize = m_perFrameSize * 2;
			if (newPerFrameSize < currentFrameBytesWritten + size) {
				newPerFrameSize = currentFrameBytesWritten + (uint32_t)size;
			}
			resize(newPerFrameSize);
			frameStart = currentFrame * m_perFrameSize;
		}

		uint32_t writeOffset = m_currentOffset;
		memcpy(m_mappedPtr + writeOffset, data, size);
		m_currentOffset += (uint32_t)size;
		return writeOffset;
	}

	void DX12VertexBuffer::resetOffset() {
		std::lock_guard<std::mutex> lock(m_appendMutex);
		auto* context = static_cast<DX12Context*>(GraphicsContext::get()->getNativeContext());
		m_currentOffset = context->getCurrentFrameIndex() * m_perFrameSize;
	}

	void DX12VertexBuffer::resize(uint32_t newPerFrameSize) {
		auto* context = static_cast<DX12Context*>(GraphicsContext::get()->getNativeContext());
		context->waitForPreviousFrame();

		uint32_t newTotalSize = newPerFrameSize * MAX_FRAMES_IN_FLIGHT;
		Microsoft::WRL::ComPtr<ID3D12Resource> newBuffer;

		CD3DX12_HEAP_PROPERTIES heapProps(D3D12_HEAP_TYPE_UPLOAD);
		CD3DX12_RESOURCE_DESC bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(newTotalSize);

		HRESULT hr = context->getDevice()->CreateCommittedResource(
			&heapProps, D3D12_HEAP_FLAG_NONE, &bufferDesc,
			D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&newBuffer)
		);
		AX_CORE_ASSERT(SUCCEEDED(hr), "Failed to resize dynamic vertex buffer");

		uint8_t* newMappedPtr = nullptr;
		D3D12_RANGE readRange = { 0, 0 };
		newBuffer->Map(0, &readRange, reinterpret_cast<void**>(&newMappedPtr));

		uint32_t currentFrame = context->getCurrentFrameIndex();
		uint32_t currentFrameBytesWritten = m_currentOffset - (currentFrame * m_perFrameSize);

		if (currentFrameBytesWritten > 0 && m_mappedPtr) {
			memcpy(newMappedPtr + (currentFrame * newPerFrameSize), m_mappedPtr + (currentFrame * m_perFrameSize), currentFrameBytesWritten);
		}

		if (m_buffer && m_mappedPtr) {
			m_buffer->Unmap(0, nullptr);
		}

		m_buffer = newBuffer;
		m_mappedPtr = newMappedPtr;
		m_perFrameSize = newPerFrameSize;
		m_size = newTotalSize;
		m_currentOffset = (currentFrame * m_perFrameSize) + currentFrameBytesWritten;

		m_view.BufferLocation = m_buffer->GetGPUVirtualAddress();
		m_view.SizeInBytes = m_size;
	}

	////////////////////////////////////////////////////////////////////////////////
	///// DX12IndexBuffer //////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////////

	// -- Static Constructor --
	DX12IndexBuffer::DX12IndexBuffer(const std::vector<uint32_t>& indices) {
		m_type = BufferType::Static;
		m_indexCount = static_cast<uint32_t>(indices.size());
		uint32_t bufferSize = m_indexCount * sizeof(uint32_t);

		CD3DX12_HEAP_PROPERTIES heapProps(D3D12_HEAP_TYPE_UPLOAD);
		CD3DX12_RESOURCE_DESC bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(bufferSize);

		// ----- Create resource -----
		auto device = static_cast<DX12Context*>(GraphicsContext::get()->getNativeContext())->getDevice();
		HRESULT hr = device->CreateCommittedResource(
			&heapProps, D3D12_HEAP_FLAG_NONE, &bufferDesc,
			D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&m_buffer)
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
		m_buffer->SetName(L"DX12 Static IndexBuffer");
		#endif
	}

	// -- Dynamic Constructor --
	DX12IndexBuffer::DX12IndexBuffer(uint32_t maxIndices) {
		m_type = BufferType::Dynamic;
		m_indexCount = 0;
		m_perFrameSize = maxIndices * sizeof(uint32_t);
		uint32_t bufferSize = m_perFrameSize * MAX_FRAMES_IN_FLIGHT;

		CD3DX12_HEAP_PROPERTIES heapProps(D3D12_HEAP_TYPE_UPLOAD);
		CD3DX12_RESOURCE_DESC bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(bufferSize);

		auto device = static_cast<DX12Context*>(GraphicsContext::get()->getNativeContext())->getDevice();
		HRESULT hr = device->CreateCommittedResource(
			&heapProps, D3D12_HEAP_FLAG_NONE, &bufferDesc,
			D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&m_buffer)
		);
		AX_THROW_IF_FAILED_HR(hr, "Failed to create dynamic index buffer");

		// ----- Upload data -----
		D3D12_RANGE readRange = { 0, 0 };
		hr = m_buffer->Map(0, &readRange, reinterpret_cast<void**>(&m_mappedPtr));
		AX_THROW_IF_FAILED_HR(hr, "Failed to map index buffer");

		// ----- Fill buffer view -----
		m_view.BufferLocation = m_buffer->GetGPUVirtualAddress();
		m_view.Format = DXGI_FORMAT_R32_UINT;
		m_view.SizeInBytes = m_perFrameSize;

		#ifdef AX_DEBUG
		m_buffer->SetName(L"DX12 Dynamic IndexBuffer");
		#endif
	}

	DX12IndexBuffer::~DX12IndexBuffer() {
		release();
	}

	void DX12IndexBuffer::release() {
		m_buffer.Reset();
		m_mappedPtr = nullptr;
	}

	void DX12IndexBuffer::bind(RenderContext* renderContext) const {
		auto* context = static_cast<DX12Context*>(GraphicsContext::get()->getNativeContext());
		auto cmdList = static_cast<DX12RenderContext*>(renderContext)->getCmdList();
		D3D12_INDEX_BUFFER_VIEW view = m_view;
		if (m_type == BufferType::Dynamic) {
			view.BufferLocation += context->getCurrentFrameIndex() * m_perFrameSize;
		}
		cmdList->IASetIndexBuffer(&view);
	}

	void DX12IndexBuffer::unbind() const {}

	void DX12IndexBuffer::update(const void* data, size_t size) {
		AX_CORE_ASSERT(m_mappedPtr, "Attempting to update a non-dynamic index buffer");
		AX_CORE_ASSERT(size <= m_view.SizeInBytes, "IndexBuffer overflow");
		if (size > m_perFrameSize) return;
		for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
			memcpy(m_mappedPtr + (i * m_perFrameSize), data, size);
		}
	}

	void DX12IndexBuffer::update(const void* data, size_t size, size_t offset) {
		AX_CORE_ASSERT(m_mappedPtr, "Attempting to update a non-dynamic index buffer");
		AX_CORE_ASSERT(offset + size <= m_view.SizeInBytes, "IndexBuffer overflow");
		for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
			memcpy(m_mappedPtr + (i * m_perFrameSize) + offset, data, size);
		}
	}

	////////////////////////////////////////////////////////////////////////////////
	///// DX12ConstantBuffer ///////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////////

	DX12ConstantBuffer::DX12ConstantBuffer(size_t size) {
		m_perFrameSize = (size + 255) & ~255;
		m_bufferSize = m_perFrameSize * MAX_FRAMES_IN_FLIGHT;

		CD3DX12_HEAP_PROPERTIES heapProps(D3D12_HEAP_TYPE_UPLOAD);
		CD3DX12_RESOURCE_DESC resDesc = CD3DX12_RESOURCE_DESC::Buffer(m_bufferSize);

		// ----- Create resource -----
		auto device = static_cast<DX12Context*>(GraphicsContext::get()->getNativeContext())->getDevice();
		HRESULT hr = device->CreateCommittedResource(
			&heapProps, D3D12_HEAP_FLAG_NONE, &resDesc,
			D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&m_buffer)
		);
		AX_THROW_IF_FAILED_HR(hr, "Failed to create constant buffer");

		// ----- Upload data -----
		D3D12_RANGE readRange = { 0, 0 };
		hr = m_buffer->Map(0, &readRange, reinterpret_cast<void**>(&m_mappedPtr));
		AX_THROW_IF_FAILED_HR(hr, "Failed to map constant buffer");

		#ifdef AX_DEBUG
		m_buffer->SetName(L"DX12 Dynamic ConstantBuffer");
		#endif
	}

	DX12ConstantBuffer::~DX12ConstantBuffer() {
		release();
	}
	
	void DX12ConstantBuffer::release() {
		m_buffer.Reset();
		m_mappedPtr = nullptr;
	}

	void DX12ConstantBuffer::bind(RenderContext* renderContext, uint32_t slot) const {
		auto* context = static_cast<DX12Context*>(GraphicsContext::get()->getNativeContext());
		auto cmdList = static_cast<DX12RenderContext*>(renderContext)->getCmdList();
		uint32_t frameOffset = context->getCurrentFrameIndex() * m_perFrameSize;
		cmdList->SetGraphicsRootConstantBufferView(slot, m_buffer->GetGPUVirtualAddress() + frameOffset);
	}

	void DX12ConstantBuffer::bind(RenderContext* renderContext, uint32_t slot, size_t offset) const {
		AX_CORE_ASSERT(offset + 256 <= m_bufferSize, "DirectX12 ConstantBuffer bounds check failed");
		auto cmdList = static_cast<DX12RenderContext*>(renderContext)->getCmdList();
		cmdList->SetGraphicsRootConstantBufferView(slot, m_buffer->GetGPUVirtualAddress() + offset);
	}

	void DX12ConstantBuffer::bindCompute(RenderContext* renderContext, uint32_t slot) const {
		auto cmdList = static_cast<DX12RenderContext*>(renderContext)->getCmdList();
		cmdList->SetComputeRootConstantBufferView(slot, m_buffer->GetGPUVirtualAddress());
	}

	void DX12ConstantBuffer::unbind() const {}

	void DX12ConstantBuffer::update(const void* data, size_t size) {
		AX_CORE_ASSERT(size <= m_perFrameSize, "ConstantBuffer overflow");
		for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
			memcpy(m_mappedPtr + (i * m_perFrameSize), data, size);
		}
	}

	uint32_t DX12ConstantBuffer::append(const void* data, size_t size) {
		std::lock_guard<std::mutex> lock(m_appendMutex);

		uint32_t alignedSize = (size + 255) & ~255;
		auto* context = static_cast<DX12Context*>(GraphicsContext::get()->getNativeContext());
		uint32_t currentFrame = context->getCurrentFrameIndex();
		uint32_t frameStart = currentFrame * m_perFrameSize;
		uint32_t currentFrameBytesWritten = m_currentOffset - frameStart;

		if (currentFrameBytesWritten + alignedSize > m_perFrameSize) {
			uint32_t newPerFrameSize = m_perFrameSize * 2;
			if (newPerFrameSize < currentFrameBytesWritten + alignedSize) {
				newPerFrameSize = currentFrameBytesWritten + alignedSize;
			}
			newPerFrameSize = (newPerFrameSize + 255) & ~255;
			resize(newPerFrameSize);
			frameStart = currentFrame * m_perFrameSize;
		}

		uint32_t writeOffset = m_currentOffset;
		memcpy(m_mappedPtr + writeOffset, data, size);
		m_currentOffset += alignedSize;
		return writeOffset;
	}

	void DX12ConstantBuffer::resetOffset() {
		std::lock_guard<std::mutex> lock(m_appendMutex);
		auto* context = static_cast<DX12Context*>(GraphicsContext::get()->getNativeContext());
		m_currentOffset = context->getCurrentFrameIndex() * m_perFrameSize;
	}

	void DX12ConstantBuffer::resize(uint32_t newPerFrameSize) {
		auto* context = static_cast<DX12Context*>(GraphicsContext::get()->getNativeContext());
		context->waitForPreviousFrame();

		uint32_t newTotalSize = newPerFrameSize * MAX_FRAMES_IN_FLIGHT;
		Microsoft::WRL::ComPtr<ID3D12Resource> newBuffer;

		CD3DX12_HEAP_PROPERTIES heapProps(D3D12_HEAP_TYPE_UPLOAD);
		CD3DX12_RESOURCE_DESC bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(newTotalSize);

		HRESULT hr = context->getDevice()->CreateCommittedResource(
			&heapProps, D3D12_HEAP_FLAG_NONE, &bufferDesc,
			D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&newBuffer)
		);
		AX_CORE_ASSERT(SUCCEEDED(hr), "Failed to resize dynamic constant buffer");

		uint8_t* newMappedPtr = nullptr;
		D3D12_RANGE readRange = { 0, 0 };
		newBuffer->Map(0, &readRange, reinterpret_cast<void**>(&newMappedPtr));

		uint32_t currentFrame = context->getCurrentFrameIndex();
		uint32_t currentFrameBytesWritten = m_currentOffset - (currentFrame * m_perFrameSize);

		if (currentFrameBytesWritten > 0 && m_mappedPtr) {
			memcpy(newMappedPtr + (currentFrame * newPerFrameSize), m_mappedPtr + (currentFrame * m_perFrameSize), currentFrameBytesWritten);
		}

		if (m_buffer && m_mappedPtr) {
			m_buffer->Unmap(0, nullptr);
		}

		m_buffer = newBuffer;
		m_mappedPtr = newMappedPtr;
		m_perFrameSize = newPerFrameSize;
		m_bufferSize = newTotalSize;
		m_currentOffset = (currentFrame * m_perFrameSize) + currentFrameBytesWritten;
	}

	////////////////////////////////////////////////////////////////////////////////
	///// DX12StructuredBuffer /////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////////

	DX12StructuredBuffer::DX12StructuredBuffer(uint32_t elementSize, uint32_t elementCount) {
		m_elementSize = elementSize;
		m_elementCount = elementCount;
		m_perFrameSize = m_elementSize * m_elementCount;
		m_bufferSize = m_perFrameSize * MAX_FRAMES_IN_FLIGHT;

		CD3DX12_HEAP_PROPERTIES heapProps(D3D12_HEAP_TYPE_UPLOAD);
		CD3DX12_RESOURCE_DESC resDesc = CD3DX12_RESOURCE_DESC::Buffer(m_bufferSize);

		// ----- Create Resource -----
		auto device = static_cast<DX12Context*>(GraphicsContext::get()->getNativeContext())->getDevice();
		HRESULT hr = device->CreateCommittedResource(
			&heapProps, D3D12_HEAP_FLAG_NONE, &resDesc,
			D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&m_buffer)
		);
		AX_THROW_IF_FAILED_HR(hr, "Failed to create structured buffer");

		// ----- Upload data -----
		D3D12_RANGE readRange = { 0, 0 };
		hr = m_buffer->Map(0, &readRange, reinterpret_cast<void**>(&m_mappedPtr));
		AX_THROW_IF_FAILED_HR(hr, "Failed to map structured buffer");

		#ifdef AX_DEBUG
		m_buffer->SetName(L"DX12 Dynamic StructuredBuffer");
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

	void DX12StructuredBuffer::bind(RenderContext* renderContext, uint32_t slot) const {
		auto* context = static_cast<DX12Context*>(GraphicsContext::get()->getNativeContext());
		auto cmdList = static_cast<DX12RenderContext*>(renderContext)->getCmdList();
		uint32_t frameOffset = context->getCurrentFrameIndex() * m_perFrameSize;
		cmdList->SetGraphicsRootShaderResourceView(slot, m_buffer->GetGPUVirtualAddress() + frameOffset);
	}

	void DX12StructuredBuffer::bind(RenderContext* renderContext, uint32_t slot, size_t offset) const {
		auto cmdList = static_cast<DX12RenderContext*>(renderContext)->getCmdList();
		cmdList->SetGraphicsRootShaderResourceView(slot, m_buffer->GetGPUVirtualAddress() + offset);
	}

	void DX12StructuredBuffer::unbind() const {}

	void DX12StructuredBuffer::update(const void* data, size_t size) {
		AX_CORE_ASSERT(size <= m_perFrameSize, "StructuredBuffer overflow");
		for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
			memcpy(m_mappedPtr + (i * m_perFrameSize), data, size);
		}
	}

	void DX12StructuredBuffer::update(const void* data, size_t size, size_t offset) {
		AX_CORE_ASSERT(offset + size <= m_perFrameSize, "StructuredBuffer overflow");
		for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
			memcpy(m_mappedPtr + (i * m_perFrameSize) + offset, data, size);
		}
	}

	uint32_t DX12StructuredBuffer::append(const void* data, size_t size) {
		std::lock_guard<std::mutex> lock(m_appendMutex);

		auto* context = static_cast<DX12Context*>(GraphicsContext::get()->getNativeContext());
		uint32_t currentFrame = context->getCurrentFrameIndex();
		uint32_t frameStart = currentFrame * m_perFrameSize;
		uint32_t currentFrameBytesWritten = m_currentOffset - frameStart;

		if (currentFrameBytesWritten + size > m_perFrameSize) {
			uint32_t newPerFrameSize = m_perFrameSize * 2;
			if (newPerFrameSize < currentFrameBytesWritten + size) {
				newPerFrameSize = currentFrameBytesWritten + (uint32_t)size;
			}
			newPerFrameSize = ((newPerFrameSize + m_elementSize - 1) / m_elementSize) * m_elementSize;
			resize(newPerFrameSize);
			frameStart = currentFrame * m_perFrameSize;
		}

		uint32_t writeOffset = m_currentOffset;
		memcpy(m_mappedPtr + writeOffset, data, size);
		m_currentOffset += (uint32_t)size;
		return writeOffset;
	}

	void DX12StructuredBuffer::resetOffset() {
		std::lock_guard<std::mutex> lock(m_appendMutex);
		auto* context = static_cast<DX12Context*>(GraphicsContext::get()->getNativeContext());
		m_currentOffset = context->getCurrentFrameIndex() * m_perFrameSize;
	}

	void DX12StructuredBuffer::resize(uint32_t newPerFrameSize) {
		auto* context = static_cast<DX12Context*>(GraphicsContext::get()->getNativeContext());
		context->waitForPreviousFrame();

		uint32_t newTotalSize = newPerFrameSize * MAX_FRAMES_IN_FLIGHT;
		Microsoft::WRL::ComPtr<ID3D12Resource> newBuffer;

		CD3DX12_HEAP_PROPERTIES heapProps(D3D12_HEAP_TYPE_UPLOAD);
		CD3DX12_RESOURCE_DESC bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(newTotalSize);

		HRESULT hr = context->getDevice()->CreateCommittedResource(
			&heapProps, D3D12_HEAP_FLAG_NONE, &bufferDesc,
			D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&newBuffer)
		);
		AX_CORE_ASSERT(SUCCEEDED(hr), "Failed to resize structured buffer");

		uint8_t* newMappedPtr = nullptr;
		D3D12_RANGE readRange = { 0, 0 };
		newBuffer->Map(0, &readRange, reinterpret_cast<void**>(&newMappedPtr));

		uint32_t currentFrame = context->getCurrentFrameIndex();
		uint32_t currentFrameBytesWritten = m_currentOffset - (currentFrame * m_perFrameSize);

		if (currentFrameBytesWritten > 0 && m_mappedPtr) {
			memcpy(newMappedPtr + (currentFrame * newPerFrameSize), m_mappedPtr + (currentFrame * m_perFrameSize), currentFrameBytesWritten);
		}

		if (m_buffer && m_mappedPtr) {
			m_buffer->Unmap(0, nullptr);
		}

		m_buffer = newBuffer;
		m_mappedPtr = newMappedPtr;
		m_perFrameSize = newPerFrameSize;
		m_bufferSize = newTotalSize;
		m_elementCount = m_perFrameSize / m_elementSize;
		m_currentOffset = (currentFrame * m_perFrameSize) + currentFrameBytesWritten;
	}

}


