#include "axpch.h"
#include "D12Texture.h"

#include "AxionEngine/Source/render/GraphicsContext.h"

#include "AxionEngine/Platform/directx/D12Context.h"

namespace Axion {


	////////////////////////////////////////////////////////////////////////////////
	///// D12Texture2D /////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////////


	D12Texture2D::D12Texture2D(const std::string& path) {

		auto* context = static_cast<D12Context*>(GraphicsContext::get()->getNativeContext());
		auto* device = context->getDevice();
		auto* cmdList = context->getCommandList();
		auto* cmdQueue = context->getCommandQueue();


		// ----- Texture loading from file -----
		int texWidth, texHeight, texChannels;
		stbi_set_flip_vertically_on_load(true);
		stbi_uc* pixels = stbi_load(path.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
		AX_CORE_ASSERT(pixels, "Failed to load texture image: {}", path);

		m_width = static_cast<uint32_t>(texWidth);
		m_height = static_cast<uint32_t>(texHeight);
		m_pixelSize = 4;


		// ----- Create the texture resource -----
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


		// ----- Upload heap -----
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

		// NOTE: Keep this, need to reset command list to work
		context->getCommandListWrapper().reset();


		// ----- Update the subresources -----
		UpdateSubresources(cmdList, m_textureResource.Get(), m_uploadHeap.Get(), 0, 0, 1, &textureData); // add data


		// ----- Transition barrier -----
		CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(
			m_textureResource.Get(),
			D3D12_RESOURCE_STATE_COPY_DEST,
			D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE
		);
		cmdList->ResourceBarrier(1, &barrier);


		// ----- Create SRV -----
		m_srvHeapIndex = context->getSrvHeapWrapper().allocate();

		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MipLevels = 1;
		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		
		auto srvCpuHandle = context->getSrvHeapWrapper().getCpuHandle(m_srvHeapIndex);
		device->CreateShaderResourceView(m_textureResource.Get(), &srvDesc, srvCpuHandle);


		// ----- Close commandList -----
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

	void D12Texture2D::unbind() const {
		// Explicit unbinding is not required in DirectX 12
	}

	void* D12Texture2D::getHandle() const {
		auto* context = static_cast<D12Context*>(GraphicsContext::get()->getNativeContext());
		return reinterpret_cast<void*>(context->getSrvHeapWrapper().getGpuHandle(m_srvHeapIndex).ptr);
	}


	////////////////////////////////////////////////////////////////////////////////
	///// D12TextureCube ///////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////////


	D12TextureCube::D12TextureCube(const std::string& filePath) {
		int texWidth, texHeight, texChannels;
		stbi_set_flip_vertically_on_load(false);
		stbi_uc* fullImage = stbi_load(filePath.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
		AX_CORE_ASSERT(fullImage, "Failed to load cubemap image: {0}", filePath);

		// ----- Determine Single Face Size -----
		m_faceWidth = texWidth / 4;
		m_faceHeight = texHeight / 3;


		// ----- Map Layout To Cubemap Faces -----
		std::array<std::pair<int, int>, 6> faceOffsets = {
			std::make_pair(2, 1), // +X
			std::make_pair(0, 1), // -X
			std::make_pair(1, 0), // +Y
			std::make_pair(1, 2), // -Y
			std::make_pair(1, 1), // +Z
			std::make_pair(3, 1)  // -Z
		};


		// ----- Load Cubemap Faces -----
		std::array<stbi_uc*, 6> pixels;
		for (int i = 0; i < 6; i++) {
			pixels[i] = new stbi_uc[m_faceWidth * m_faceHeight * m_pixelSize];
			int offsetX = faceOffsets[i].first * m_faceWidth;
			int offsetY = faceOffsets[i].second * m_faceHeight;

			for (uint32_t y = 0; y < m_faceHeight; y++) {
				memcpy(
					pixels[i] + y * m_faceWidth * m_pixelSize,
					fullImage + ((offsetY + y) * texWidth + offsetX) * m_pixelSize,
					m_faceWidth * m_pixelSize
				);
			}
		}

		stbi_image_free(fullImage);


		// ----- Setup common GPU Resources -----
		setupGpuResources(pixels);


		// ----- Clean up -----
		for (int i = 0; i < 6; i++) {
			stbi_image_free(pixels[i]);
		}
	}

	D12TextureCube::D12TextureCube(const std::array<std::string, 6>& paths) {
		int texWidth, texHeight, texChannels;
		stbi_set_flip_vertically_on_load(false);
		std::array<stbi_uc*, 6> pixels = {};
		

		// ----- Load Cubemap Faces -----
		for (int i = 0; i < 6; i++) {
			pixels[i] = stbi_load(paths[i].c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
			AX_CORE_ASSERT(pixels[i], "Failed to load cubemap face: {0}", paths[i]);
		}

		m_faceWidth = static_cast<uint32_t>(texWidth);
		m_faceHeight = static_cast<uint32_t>(texHeight);


		// ----- Setup common GPU Resources -----
		setupGpuResources(pixels);


		// ----- Clean up -----
		for (int i = 0; i < 6; i++) {
			stbi_image_free(pixels[i]);
		}

	}

	D12TextureCube::~D12TextureCube() {
		release();
	}

	void D12TextureCube::release() {
		m_textureResource.Reset();
		m_uploadHeap.Reset();
	}

	void D12TextureCube::bind() const {
		auto* context = static_cast<D12Context*>(GraphicsContext::get()->getNativeContext());
		auto* cmdList = context->getCommandList();

		ID3D12DescriptorHeap* ppHeaps[] = { context->getSrvHeapWrapper().getHeap() };
		cmdList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);

		auto srvGpuHandle = context->getSrvHeapWrapper().getGpuHandle(m_srvHeapIndex);
		cmdList->SetGraphicsRootDescriptorTable(2, srvGpuHandle); // make 2 configurable
	}

	void D12TextureCube::unbind() const {
		// Explicit unbinding is not required in DirectX 12
	}

	void* D12TextureCube::getHandle() const {
		auto* context = static_cast<D12Context*>(GraphicsContext::get()->getNativeContext());
		return reinterpret_cast<void*>(context->getSrvHeapWrapper().getGpuHandle(m_srvHeapIndex).ptr);
	}

	void D12TextureCube::setupGpuResources(const std::array<stbi_uc*, 6>& pixels) {
		auto* context = static_cast<D12Context*>(GraphicsContext::get()->getNativeContext());
		auto* device = context->getDevice();
		auto* cmdList = context->getCommandList();
		auto* cmdQueue = context->getCommandQueue();


		// ----- Texture resource -----
		D3D12_RESOURCE_DESC texDesc = {};
		texDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		texDesc.Width = m_faceWidth;
		texDesc.Height = m_faceHeight;
		texDesc.DepthOrArraySize = 6; // 6 faces
		texDesc.MipLevels = 1;
		texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		texDesc.SampleDesc.Count = 1;
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
		AX_THROW_IF_FAILED_HR(hr, "Failed to create cubemap texture resource");


		// ----- Upload buffer -----
		const UINT64 uploadBufferSize = GetRequiredIntermediateSize(m_textureResource.Get(), 0, 6);
		CD3DX12_HEAP_PROPERTIES uploadProps(D3D12_HEAP_TYPE_UPLOAD);
		CD3DX12_RESOURCE_DESC uploadHeapDesc = CD3DX12_RESOURCE_DESC::Buffer(uploadBufferSize);
		hr = device->CreateCommittedResource(
			&uploadProps,
			D3D12_HEAP_FLAG_NONE,
			&uploadHeapDesc,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&m_uploadHeap)
		);
		AX_THROW_IF_FAILED_HR(hr, "Failed to create cubemap upload heap");


		// ----- Subresources -----
		std::array<D3D12_SUBRESOURCE_DATA, 6> subresources;
		for (int i = 0; i < 6; i++) {
			subresources[i].pData = pixels[i];
			subresources[i].RowPitch = m_faceWidth * m_pixelSize;
			subresources[i].SlicePitch = subresources[i].RowPitch * m_faceHeight;
		}

		context->getCommandListWrapper().reset();
		UpdateSubresources(cmdList, m_textureResource.Get(), m_uploadHeap.Get(), 0, 0, 6, subresources.data());


		// ----- Transition -----
		CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(
			m_textureResource.Get(),
			D3D12_RESOURCE_STATE_COPY_DEST,
			D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE
		);
		cmdList->ResourceBarrier(1, &barrier);


		// ----- SRV -----
		m_srvHeapIndex = context->getSrvHeapWrapper().allocate();

		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srvDesc.TextureCube.MipLevels = 1;

		auto srvCpuHandle = context->getSrvHeapWrapper().getCpuHandle(m_srvHeapIndex);
		device->CreateShaderResourceView(m_textureResource.Get(), &srvDesc, srvCpuHandle);


		// ----- Execute -----
		AX_THROW_IF_FAILED_HR(cmdList->Close(), "Failed to close command list for cubemap");
		ID3D12CommandList* cmdLists[] = { cmdList };
		cmdQueue->ExecuteCommandLists(_countof(cmdLists), cmdLists);
		context->waitForPreviousFrame();
	}

}
