#include "axpch.h"
#include "D12Texture.h"

#include "AxionEngine/Source/render/GraphicsContext.h"

#include "AxionEngine/Platform/directx/D12Context.h"

#include "AxionEngine/Vendor/stb_image/stb_image.h"

namespace Axion {

	D12Texture2D::D12Texture2D(const std::string& path) {

		auto* context = static_cast<D12Context*>(GraphicsContext::get()->getNativeContext());
		auto* device = context->getDevice();
		auto* cmdList = context->getCommandList();
		auto* cmdQueue = context->getCommandQueue();

		// texture loading from file
		int texWidth, texHeight, texChannels;
		stbi_set_flip_vertically_on_load(true);
		stbi_uc* pixels = stbi_load(path.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
		AX_CORE_ASSERT(pixels, "Failed to load texture image: {0}", path);

		m_width = static_cast<uint32_t>(texWidth);
		m_height = static_cast<uint32_t>(texHeight);
		m_pixelSize = 4;

		// create the texture resource
		D3D12_RESOURCE_DESC texDesc = {};
		texDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		texDesc.Width = m_width;
		texDesc.Height = m_height;
		texDesc.DepthOrArraySize = 1;
		texDesc.MipLevels = 1;
		texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		texDesc.SampleDesc.Count = 1;
		texDesc.SampleDesc.Quality = 0;
		texDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
		texDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

		CD3DX12_HEAP_PROPERTIES texProps(D3D12_HEAP_TYPE_DEFAULT);
		HRESULT hr = device->CreateCommittedResource(
			&texProps,
			D3D12_HEAP_FLAG_NONE,
			&texDesc,
			D3D12_RESOURCE_STATE_COPY_DEST,
			nullptr,
			IID_PPV_ARGS(&m_textureResource)
		);
		AX_THROW_IF_FAILED_HR(hr, "Failed to create create texture resource");

		// upload heap
		const UINT64 uploadBufferSize = GetRequiredIntermediateSize(m_textureResource.Get(), 0, 1);
		CD3DX12_RESOURCE_DESC uploadHeapDesc = CD3DX12_RESOURCE_DESC::Buffer(uploadBufferSize);
		CD3DX12_HEAP_PROPERTIES uploadProps(D3D12_HEAP_TYPE_UPLOAD);
		hr = device->CreateCommittedResource(
			&uploadProps,
			D3D12_HEAP_FLAG_NONE,
			&uploadHeapDesc,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&m_uploadHeap)
		);

		D3D12_SUBRESOURCE_DATA textureData = {};
		textureData.pData = pixels;
		textureData.RowPitch = m_width * m_pixelSize;
		textureData.SlicePitch = textureData.RowPitch * m_height;

		// keep this, need to reset command list to work
		context->getCommandListWrapper().reset();

		// UpdateSubresources
		UpdateSubresources(cmdList, m_textureResource.Get(), m_uploadHeap.Get(), 0, 0, 1, &textureData); // add data

		// transition
		CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(
			m_textureResource.Get(),
			D3D12_RESOURCE_STATE_COPY_DEST,
			D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE
		);
		cmdList->ResourceBarrier(1, &barrier);

		// create srv
		m_srvHeapIndex = context->getSrvHeapWrapper().allocate();

		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MipLevels = 1;
		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		
		auto srvCpuHandle = context->getSrvHeapWrapper().getCpuHandle(m_srvHeapIndex);
		device->CreateShaderResourceView(m_textureResource.Get(), &srvDesc, srvCpuHandle);

		// close cmdlist
		AX_THROW_IF_FAILED_HR(cmdList->Close(), "Failed to close the command list");
		ID3D12CommandList* cmdLists[] = { cmdList };
		cmdQueue->ExecuteCommandLists(_countof(cmdLists), cmdLists);

		context->waitForPreviousFrame();

		stbi_image_free(pixels);
	}

	D12Texture2D::~D12Texture2D() {
		release();
	}

	void D12Texture2D::release() {
		m_textureResource.Reset();
		m_uploadHeap.Reset();
	}

	void D12Texture2D::bind() const {
		auto* context = static_cast<D12Context*>(GraphicsContext::get()->getNativeContext());
		auto* cmdList = context->getCommandList();

		ID3D12DescriptorHeap* ppHeaps[] = { context->getSrvHeapWrapper().getHeap() };
		cmdList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);

		auto srvGpuHandle = context->getSrvHeapWrapper().getGpuHandle(m_srvHeapIndex);
		cmdList->SetGraphicsRootDescriptorTable(2, srvGpuHandle);
	}

	// not required
	void D12Texture2D::unbind() const {}

	void* D12Texture2D::getHandle() const {
		auto* context = static_cast<D12Context*>(GraphicsContext::get()->getNativeContext());
		return reinterpret_cast<void*>(context->getSrvHeapWrapper().getGpuHandle(m_srvHeapIndex).ptr);
	}

}
