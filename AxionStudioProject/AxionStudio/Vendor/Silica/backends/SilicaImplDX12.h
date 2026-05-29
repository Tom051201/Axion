#pragma once

#include <d3d12.h>

#include "../include/Renderer.h"

namespace Silica {

	bool ImplDX12_init(ID3D12Device* device, UINT numFramesInFlight, DXGI_FORMAT rtvFormat);

	void ImplDX12_shutdown();

	void ImplDX12_newFrame();

	void ImplDX12_renderDrawData(const DrawList* drawData, ID3D12GraphicsCommandList* commandList, float screenWidth, float screenHeight);

	void ImplDX12_uploadFontAtlas(ID3D12GraphicsCommandList* cmdList, const uint8_t* pixels, uint32_t width, uint32_t height);

	TextureID ImplDX12_registerTexture(ID3D12Resource* textureResource);

}
