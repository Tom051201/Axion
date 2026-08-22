#pragma once

#include <d3d11.h>
#include <Silica/include/Renderer.h>

namespace Silica {

	bool ImplDX11_init(ID3D11Device* device, ID3D11DeviceContext* deviceContext);
	void ImplDX11_shutdown();
	void ImplDX11_newFrame();
	void ImplDX11_renderDrawData(const DrawList* drawData, float screenWidth, float screenHeight);
	void ImplDX11_uploadFontAtlas(const uint8_t* pixels, uint32_t width, uint32_t height);

	TextureID ImplDX11_registerTexture(ID3D11ShaderResourceView* srv);
	void ImplDX11_updateTexture(TextureID id, ID3D11ShaderResourceView* srv);

}
