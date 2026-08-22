#include "axpch.h"
#include "DX11Buffers.h"

#include <d3d11_1.h>

#include "AxionEngine/Platform/directx11/DX11Context.h"
#include "AxionEngine/Platform/directx11/DX11DebugLayer.h"

namespace Axion {

	////////////////////////////////////////////////////////////////////////////////
	///// DX11VertexBuffer /////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////////

	DX11VertexBuffer::DX11VertexBuffer(const void* data, uint32_t size, uint32_t stride) {
		m_type = BufferType::Static;
		m_stride = stride;
		m_size = size;
		m_vertexCount = m_size / m_stride;

		D3D11_BUFFER_DESC desc = {};
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.ByteWidth = m_size;
		desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

		D3D11_SUBRESOURCE_DATA initData = {};
		initData.pSysMem = data;

		auto device = static_cast<DX11Context*>(GraphicsContext::get()->getNativeContext())->getDevice();
		HRESULT hr = device->CreateBuffer(&desc, &initData, &m_buffer);
		AX_THROW_IF_FAILED_HR(hr, "Failed to create static vertex buffer");

		#ifdef AX_DEBUG
		DX11DebugLayer::setName(m_buffer.Get(), "DX11 Static VertexBuffer");
		#endif
	}

	DX11VertexBuffer::DX11VertexBuffer(uint32_t size, uint32_t stride) {
		m_type = BufferType::Dynamic;
		m_size = size;
		m_stride = stride;
		m_localData.resize(m_size);

		D3D11_BUFFER_DESC desc = {};
		desc.Usage = D3D11_USAGE_DYNAMIC;
		desc.ByteWidth = m_size;
		desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

		auto device = static_cast<DX11Context*>(GraphicsContext::get()->getNativeContext())->getDevice();
		HRESULT hr = device->CreateBuffer(&desc, nullptr, &m_buffer);
		AX_THROW_IF_FAILED_HR(hr, "Failed to create dynamic vertex buffer");

		#ifdef AX_DEBUG
		DX11DebugLayer::setName(m_buffer.Get(), "DX11 Dynamic VertexBuffer");
		#endif
	}

	DX11VertexBuffer::~DX11VertexBuffer() {
		release();
	}

	void DX11VertexBuffer::release() {
		m_buffer.Reset();
		m_localData.clear();
	}

	void DX11VertexBuffer::bind(uint32_t slot, uint32_t offset) const {
		auto ctx = static_cast<DX11Context*>(GraphicsContext::get()->getNativeContext())->getDeviceContext();

		if (m_type == BufferType::Dynamic && m_isDirty && m_currentOffset > 0) {
			D3D11_MAPPED_SUBRESOURCE mapped;
			HRESULT hr = ctx->Map(m_buffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
			AX_THROW_IF_FAILED_HR(hr, "Failed to map DX11 VertexBuffer");
			memcpy(mapped.pData, m_localData.data(), m_currentOffset);
			ctx->Unmap(m_buffer.Get(), 0);
			m_isDirty = false;
		}

		UINT stride = m_stride;
		UINT d3dOffset = offset;
		ctx->IASetVertexBuffers(slot, 1, m_buffer.GetAddressOf(), &stride, &d3dOffset);
	}

	void DX11VertexBuffer::unbind() const {}

	void DX11VertexBuffer::update(const void* data, size_t size) {
		AX_CORE_ASSERT(size <= m_size, "DX11 VertexBuffer overflow!");
		memcpy(m_localData.data(), data, size);
		m_isDirty = true;
	}

	void DX11VertexBuffer::update(const void* data, size_t size, size_t offset) {
		AX_CORE_ASSERT(offset + size <= m_size, "DX11 VertexBuffer overflow!");
		memcpy(m_localData.data() + offset, data, size);
		m_isDirty = true;
	}

	uint32_t DX11VertexBuffer::append(const void* data, size_t size) {
		AX_CORE_ASSERT(m_currentOffset + size <= m_size, "DX11 VertexBuffer overflow!");
		uint32_t writeOffset = m_currentOffset;
		memcpy(m_localData.data() + writeOffset, data, size);
		m_currentOffset += (uint32_t)size;
		m_isDirty = true;
		return writeOffset;
	}

	void DX11VertexBuffer::resetOffset() {
		m_currentOffset = 0;
	}

	////////////////////////////////////////////////////////////////////////////////
	///// DX11IndexBuffer //////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////////

	DX11IndexBuffer::DX11IndexBuffer(const std::vector<uint32_t>& indices) {
		m_type = BufferType::Static;
		m_indexCount = (uint32_t)indices.size();
		m_size = m_indexCount * sizeof(uint32_t);

		D3D11_BUFFER_DESC desc = {};
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.ByteWidth = m_size;
		desc.BindFlags = D3D11_BIND_INDEX_BUFFER;

		D3D11_SUBRESOURCE_DATA initData = {};
		initData.pSysMem = indices.data();

		auto device = static_cast<DX11Context*>(GraphicsContext::get()->getNativeContext())->getDevice();
		HRESULT hr = device->CreateBuffer(&desc, &initData, &m_buffer);
		AX_THROW_IF_FAILED_HR(hr, "Failed to create static index buffer");

		#ifdef AX_DEBUG
		DX11DebugLayer::setName(m_buffer.Get(), "DX11 Static IndexBuffer");
		#endif
	}

	DX11IndexBuffer::DX11IndexBuffer(uint32_t maxIndices) {
		m_type = BufferType::Dynamic;
		m_size = maxIndices * sizeof(uint32_t);
		m_localData.resize(m_size);

		D3D11_BUFFER_DESC desc = {};
		desc.Usage = D3D11_USAGE_DYNAMIC;
		desc.ByteWidth = m_size;
		desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
		desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

		auto device = static_cast<DX11Context*>(GraphicsContext::get()->getNativeContext())->getDevice();
		HRESULT hr = device->CreateBuffer(&desc, nullptr, &m_buffer);
		AX_THROW_IF_FAILED_HR(hr, "Failed to create dynamic index buffer");

		#ifdef AX_DEBUG
		DX11DebugLayer::setName(m_buffer.Get(), "DX11 Dynamic IndexBuffer");
		#endif
	}

	DX11IndexBuffer::~DX11IndexBuffer() {
		release();
	}

	void DX11IndexBuffer::release() {
		m_buffer.Reset();
		m_localData.clear();
	}

	void DX11IndexBuffer::bind() const {
		auto ctx = static_cast<DX11Context*>(GraphicsContext::get()->getNativeContext())->getDeviceContext();

		if (m_type == BufferType::Dynamic && m_isDirty) {
			D3D11_MAPPED_SUBRESOURCE mapped;
			HRESULT hr = ctx->Map(m_buffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
			AX_THROW_IF_FAILED_HR(hr, "Failed to map DX11 IndexBuffer");
			memcpy(mapped.pData, m_localData.data(), m_localData.size());
			ctx->Unmap(m_buffer.Get(), 0);
			m_isDirty = false;
		}

		ctx->IASetIndexBuffer(m_buffer.Get(), DXGI_FORMAT_R32_UINT, 0);
	}

	void DX11IndexBuffer::unbind() const {}

	void DX11IndexBuffer::update(const void* data, size_t size) {
		AX_CORE_ASSERT(size <= m_size, "DX11 IndexBuffer overflow!");
		memcpy(m_localData.data(), data, size);
		m_isDirty = true;
	}

	void DX11IndexBuffer::update(const void* data, size_t size, size_t offset) {
		AX_CORE_ASSERT(offset + size <= m_size, "DX11 IndexBuffer overflow!");
		memcpy(m_localData.data() + offset, data, size);
		m_isDirty = true;
	}

	////////////////////////////////////////////////////////////////////////////////
	///// DX11ConstantBuffer ///////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////////

	DX11ConstantBuffer::DX11ConstantBuffer(size_t size)
		: m_bufferSize((size + 255) & ~255) {

		m_localData.resize(m_bufferSize);

		D3D11_BUFFER_DESC desc = {};
		desc.Usage = D3D11_USAGE_DYNAMIC;
		desc.ByteWidth = (UINT)m_bufferSize;
		desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

		auto device = static_cast<DX11Context*>(GraphicsContext::get()->getNativeContext())->getDevice();
		HRESULT hr = device->CreateBuffer(&desc, nullptr, &m_buffer);
		AX_THROW_IF_FAILED_HR(hr, "Failed to create constant buffer");

		#ifdef AX_DEBUG
		DX11DebugLayer::setName(m_buffer.Get(), "DX11 Dynamic ConstantBuffer");
		#endif
	}

	DX11ConstantBuffer::~DX11ConstantBuffer() {
		release();
	}

	void DX11ConstantBuffer::release() {
		m_buffer.Reset();
		m_localData.clear();
	}

	void DX11ConstantBuffer::bind(uint32_t slot) const {
		bind(slot, 0);
	}

	void DX11ConstantBuffer::bind(uint32_t slot, size_t offset) const {
		auto ctx = static_cast<DX11Context*>(GraphicsContext::get()->getNativeContext())->getDeviceContext();

		if (m_isDirty && m_currentOffset > 0) {
			D3D11_MAPPED_SUBRESOURCE mapped;
			HRESULT hr = ctx->Map(m_buffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
			AX_THROW_IF_FAILED_HR(hr, "Failed to map DX11 ConstantBuffer")
			memcpy(mapped.pData, m_localData.data(), m_currentOffset);
			ctx->Unmap(m_buffer.Get(), 0);
			m_isDirty = false;
		}

		Microsoft::WRL::ComPtr<ID3D11DeviceContext1> ctx1;
		if (SUCCEEDED(ctx->QueryInterface(IID_PPV_ARGS(&ctx1)))) {
			UINT first = (UINT)offset / 16;
			UINT num = (UINT)(m_bufferSize - offset) / 16;
			if (num > 4096) num = 4096;

			ctx1->VSSetConstantBuffers1(slot, 1, m_buffer.GetAddressOf(), &first, &num);
			ctx1->PSSetConstantBuffers1(slot, 1, m_buffer.GetAddressOf(), &first, &num);
		}
		else {
			ctx->VSSetConstantBuffers(slot, 1, m_buffer.GetAddressOf());
			ctx->PSSetConstantBuffers(slot, 1, m_buffer.GetAddressOf());
		}
	}

	void DX11ConstantBuffer::unbind() const {}

	void DX11ConstantBuffer::update(const void* data, size_t size) {
		AX_CORE_ASSERT(size <= m_bufferSize, "DX11 ConstantBuffer overflow!");
		memcpy(m_localData.data(), data, size);
		m_isDirty = true;
	}

	uint32_t DX11ConstantBuffer::append(const void* data, size_t size) {
		uint32_t alignedSize = (size + 255) & ~255;
		AX_CORE_ASSERT(m_currentOffset + alignedSize <= m_bufferSize, "DX11 ConstantBuffer overflow!");
		uint32_t writeOffset = m_currentOffset;
		memcpy(m_localData.data() + writeOffset, data, size);
		m_currentOffset += alignedSize;
		m_isDirty = true;
		return writeOffset;
	}

	void DX11ConstantBuffer::resetOffset() {
		m_currentOffset = 0;
	}

	////////////////////////////////////////////////////////////////////////////////
	///// DX11StructuredBuffer /////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////////

	DX11StructuredBuffer::DX11StructuredBuffer(uint32_t elementSize, uint32_t elementCount)
		: m_elementSize(elementSize), m_elementCount(elementCount) {

		m_bufferSize = m_elementSize * m_elementCount;
		m_localData.resize(m_bufferSize);

		D3D11_BUFFER_DESC desc = {};
		desc.Usage = D3D11_USAGE_DYNAMIC;
		desc.ByteWidth = m_bufferSize;
		desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		desc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
		desc.StructureByteStride = m_elementSize;

		auto device = static_cast<DX11Context*>(GraphicsContext::get()->getNativeContext())->getDevice();
		HRESULT hr = device->CreateBuffer(&desc, nullptr, &m_buffer);
		AX_THROW_IF_FAILED_HR(hr, "Failed to create structured buffer");

		#ifdef AX_DEBUG
		DX11DebugLayer::setName(m_buffer.Get(), "DX11 Dynamic StructuredBuffer");
		#endif
	}

	DX11StructuredBuffer::~DX11StructuredBuffer() {
		release();
	}

	void DX11StructuredBuffer::release() {
		m_buffer.Reset();
		m_srvs.clear();
		m_localData.clear();
	}

	void DX11StructuredBuffer::bind(uint32_t slot) const {
		bind(slot, 0);
	}

	void DX11StructuredBuffer::bind(uint32_t slot, size_t offset) const {
		auto device = static_cast<DX11Context*>(GraphicsContext::get()->getNativeContext())->getDevice();
		auto ctx = static_cast<DX11Context*>(GraphicsContext::get()->getNativeContext())->getDeviceContext();

		if (m_isDirty && m_currentOffset > 0) {
			D3D11_MAPPED_SUBRESOURCE mapped;
			HRESULT hr = ctx->Map(m_buffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
			AX_THROW_IF_FAILED_HR(hr, "Failed to map DX11 Buffer");
			memcpy(mapped.pData, m_localData.data(), m_currentOffset);
			ctx->Unmap(m_buffer.Get(), 0);
			m_isDirty = false;
		}

		if (m_srvs.find(offset) == m_srvs.end()) {
			D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
			srvDesc.Format = DXGI_FORMAT_UNKNOWN;
			srvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
			srvDesc.Buffer.FirstElement = (UINT)(offset / m_elementSize);

			UINT elementsAppended = (UINT)(m_currentOffset / m_elementSize);
			if (elementsAppended == 0) elementsAppended = m_elementCount;
			srvDesc.Buffer.NumElements = elementsAppended - srvDesc.Buffer.FirstElement;

			HRESULT hr = device->CreateShaderResourceView(m_buffer.Get(), &srvDesc, &m_srvs[offset]);
			AX_THROW_IF_FAILED_HR(hr, "Failed to create DX11 StructuredBuffer SRV");
		}

		ctx->VSSetShaderResources(slot, 1, m_srvs[offset].GetAddressOf());

		#ifdef AX_DEBUG
		DX11DebugLayer::setName(m_srvs[offset].Get(), "DX11 StructuredBuffer SRV");
		#endif
	}

	void DX11StructuredBuffer::unbind() const {}

	void DX11StructuredBuffer::update(const void* data, size_t size) {
		AX_CORE_ASSERT(size <= m_bufferSize, "DX11 StructuredBuffer overflow!");
		memcpy(m_localData.data(), data, size);
		m_isDirty = true;
	}

	void DX11StructuredBuffer::update(const void* data, size_t size, size_t offset) {
		AX_CORE_ASSERT(offset + size <= m_bufferSize, "DX11 StructuredBuffer overflow!");
		memcpy(m_localData.data() + offset, data, size);
		m_isDirty = true;
	}

	uint32_t DX11StructuredBuffer::append(const void* data, size_t size) {
		AX_CORE_ASSERT(m_currentOffset + size <= m_bufferSize, "DX11 StructuredBuffer overflow!");
		uint32_t writeOffset = m_currentOffset;
		memcpy(m_localData.data() + writeOffset, data, size);
		m_currentOffset += (uint32_t)size;
		m_isDirty = true;
		return writeOffset;
	}

	void DX11StructuredBuffer::resetOffset() {
		m_currentOffset = 0;
	}

}
