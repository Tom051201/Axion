#include "SilicaImplDX11.h"

#include <stdexcept>
#include <unordered_map>
#include <d3dcompiler.h>
#include <wrl/client.h>

using Microsoft::WRL::ComPtr;

namespace Silica {

	struct BackendStateDX11 {
		ID3D11Device* device = nullptr;
		ID3D11DeviceContext* deviceContext = nullptr;

		ComPtr<ID3D11VertexShader> vertexShader;
		ComPtr<ID3D11PixelShader> pixelShader;
		ComPtr<ID3D11InputLayout> inputLayout;
		ComPtr<ID3D11Buffer> constantBuffer;

		ComPtr<ID3D11BlendState> blendState;
		ComPtr<ID3D11RasterizerState> rasterizerState;
		ComPtr<ID3D11DepthStencilState> depthStencilState;
		ComPtr<ID3D11SamplerState> fontSampler;

		ComPtr<ID3D11Buffer> vertexBuffer;
		ComPtr<ID3D11Buffer> indexBuffer;

		uint32_t maxVertices = 200000;
		uint32_t maxIndices = 600000;

		ComPtr<ID3D11Texture2D> fontTexture;
		ComPtr<ID3D11ShaderResourceView> fontSrv;

		uint32_t nextAllocatedTextureID = 1;
		std::unordered_map<TextureID, ID3D11ShaderResourceView*> textureMap;
	};

	static BackendStateDX11 g_state;

	static ComPtr<ID3DBlob> compileShader(const char* shaderCode, const char* entryPoint, const char* target) {
		ComPtr<ID3DBlob> shaderBlob;
		ComPtr<ID3DBlob> errorBlob;
		if (FAILED(D3DCompile(shaderCode, strlen(shaderCode), nullptr, nullptr, nullptr, entryPoint, target, D3DCOMPILE_ENABLE_STRICTNESS, 0, &shaderBlob, &errorBlob))) {
			if (errorBlob) OutputDebugStringA((char*)errorBlob->GetBufferPointer());
			throw std::runtime_error("Failed to compile Silica DX11 UI Shader!");
		}
		return shaderBlob;
	}

	bool ImplDX11_init(ID3D11Device* device, ID3D11DeviceContext* deviceContext) {
		g_state.device = device;
		g_state.deviceContext = deviceContext;

		const char* shaderSource = R"(
			cbuffer RootConstants : register(b0) { float4x4 ProjectionMatrix; };
			struct VS_INPUT { float2 pos : POSITION; float2 uv : TEXCOORD0; uint color : COLOR0; };
			Texture2D fontTex : register(t0);
			SamplerState fontSampler : register(s0);
			struct PS_INPUT { float4 position : SV_POSITION; float2 uv : TEXCOORD; float4 color : COLOR; };

			PS_INPUT VSMain(VS_INPUT input) {
				PS_INPUT output;
				output.position = mul(ProjectionMatrix, float4(input.pos.xy, 0.0f, 1.0f));
				output.uv = input.uv;
				output.color = float4((input.color & 0xFF) / 255.0f, ((input.color >> 8) & 0xFF) / 255.0f, ((input.color >> 16) & 0xFF) / 255.0f, ((input.color >> 24) & 0xFF) / 255.0f);
				return output;
			}

			float4 PSMain(PS_INPUT input) : SV_Target {
				float4 finalColor = input.color;
				if (input.uv.x < -0.5f) {
					float thickness = -input.uv.x;
					float halfThickness = thickness * 0.5f;
					float pixelDist = abs(input.uv.y - 0.5f) * thickness;
					float edgeFade = 1.0f - smoothstep(halfThickness - 1.0f, halfThickness, pixelDist);
					finalColor.a *= edgeFade;
				} else {
					finalColor *= fontTex.Sample(fontSampler, input.uv);
				}
				return finalColor;
			}
		)";

		auto vsBlob = compileShader(shaderSource, "VSMain", "vs_5_0");
		auto psBlob = compileShader(shaderSource, "PSMain", "ps_5_0");

		device->CreateVertexShader(vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), nullptr, &g_state.vertexShader);
		device->CreatePixelShader(psBlob->GetBufferPointer(), psBlob->GetBufferSize(), nullptr, &g_state.pixelShader);

		D3D11_INPUT_ELEMENT_DESC inputElementDescs[] = {
			{ "POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, offsetof(Vertex, position), D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, offsetof(Vertex, uv), D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "COLOR",    0, DXGI_FORMAT_R32_UINT,     0, offsetof(Vertex, color), D3D11_INPUT_PER_VERTEX_DATA, 0 },
		};
		device->CreateInputLayout(inputElementDescs, 3, vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), &g_state.inputLayout);

		// States
		D3D11_BLEND_DESC blendDesc = {};
		blendDesc.RenderTarget[0].BlendEnable = TRUE;
		blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
		blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
		blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
		blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
		blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA;
		blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
		blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
		device->CreateBlendState(&blendDesc, &g_state.blendState);

		D3D11_RASTERIZER_DESC rasterDesc = {};
		rasterDesc.FillMode = D3D11_FILL_SOLID;
		rasterDesc.CullMode = D3D11_CULL_NONE;
		rasterDesc.ScissorEnable = TRUE;
		device->CreateRasterizerState(&rasterDesc, &g_state.rasterizerState);

		D3D11_DEPTH_STENCIL_DESC depthDesc = {};
		depthDesc.DepthEnable = FALSE;
		device->CreateDepthStencilState(&depthDesc, &g_state.depthStencilState);

		D3D11_SAMPLER_DESC samplerDesc = {};
		samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
		samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
		samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
		device->CreateSamplerState(&samplerDesc, &g_state.fontSampler);

		// Buffers
		D3D11_BUFFER_DESC cbDesc = { sizeof(float) * 16, D3D11_USAGE_DYNAMIC, D3D11_BIND_CONSTANT_BUFFER, D3D11_CPU_ACCESS_WRITE, 0, 0 };
		device->CreateBuffer(&cbDesc, nullptr, &g_state.constantBuffer);

		D3D11_BUFFER_DESC vbDesc = { g_state.maxVertices * sizeof(Vertex), D3D11_USAGE_DYNAMIC, D3D11_BIND_VERTEX_BUFFER, D3D11_CPU_ACCESS_WRITE, 0, 0 };
		device->CreateBuffer(&vbDesc, nullptr, &g_state.vertexBuffer);

		D3D11_BUFFER_DESC ibDesc = { g_state.maxIndices * sizeof(uint32_t), D3D11_USAGE_DYNAMIC, D3D11_BIND_INDEX_BUFFER, D3D11_CPU_ACCESS_WRITE, 0, 0 };
		device->CreateBuffer(&ibDesc, nullptr, &g_state.indexBuffer);

		return true;
	}

	void ImplDX11_shutdown() {
		g_state.textureMap.clear();
		g_state = BackendStateDX11{};
	}

	void ImplDX11_newFrame() {}

	void ImplDX11_renderDrawData(const DrawList* drawData, float screenWidth, float screenHeight) {
		if (!drawData || drawData->vertices.empty() || drawData->indices.empty()) return;

		auto ctx = g_state.deviceContext;
		size_t vertexCount = std::min((size_t)g_state.maxVertices, drawData->vertices.size());
		size_t indexCount = std::min((size_t)g_state.maxIndices, drawData->indices.size());

		// Map Buffers
		D3D11_MAPPED_SUBRESOURCE mappedVB, mappedIB, mappedCB;
		ctx->Map(g_state.vertexBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedVB);
		memcpy(mappedVB.pData, drawData->vertices.data(), vertexCount * sizeof(Vertex));
		ctx->Unmap(g_state.vertexBuffer.Get(), 0);

		ctx->Map(g_state.indexBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedIB);
		memcpy(mappedIB.pData, drawData->indices.data(), indexCount * sizeof(uint32_t));
		ctx->Unmap(g_state.indexBuffer.Get(), 0);

		ctx->Map(g_state.constantBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedCB);
		float L = 0.0f, R = screenWidth, T = 0.0f, B = screenHeight;
		float mvp[4][4] = {
			{ 2.0f / (R - L), 0.0f, 0.0f, 0.0f },
			{ 0.0f, 2.0f / (T - B), 0.0f, 0.0f },
			{ 0.0f, 0.0f, 0.5f, 0.0f },
			{ (L + R) / (L - R), (T + B) / (B - T), 0.5f, 1.0f },
		};
		memcpy(mappedCB.pData, mvp, sizeof(mvp));
		ctx->Unmap(g_state.constantBuffer.Get(), 0);

		// Bind State
		ctx->IASetInputLayout(g_state.inputLayout.Get());
		UINT stride = sizeof(Vertex), offset = 0;
		ctx->IASetVertexBuffers(0, 1, g_state.vertexBuffer.GetAddressOf(), &stride, &offset);
		ctx->IASetIndexBuffer(g_state.indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
		ctx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		D3D11_VIEWPORT vp = {};
		vp.TopLeftX = 0.0f;
		vp.TopLeftY = 0.0f;
		vp.Width = screenWidth;
		vp.Height = screenHeight;
		vp.MinDepth = 0.0f;
		vp.MaxDepth = 1.0f;
		ctx->RSSetViewports(1, &vp);

		ctx->VSSetShader(g_state.vertexShader.Get(), nullptr, 0);
		ctx->VSSetConstantBuffers(0, 1, g_state.constantBuffer.GetAddressOf());

		ctx->PSSetShader(g_state.pixelShader.Get(), nullptr, 0);
		ctx->PSSetSamplers(0, 1, g_state.fontSampler.GetAddressOf());

		ctx->OMSetBlendState(g_state.blendState.Get(), nullptr, 0xffffffff);
		ctx->OMSetDepthStencilState(g_state.depthStencilState.Get(), 0);
		ctx->RSSetState(g_state.rasterizerState.Get());

		// Draw Commands
		for (const auto& cmd : drawData->commands) {
			if (cmd.indexCount == 0 || cmd.clipRect.right <= cmd.clipRect.left || cmd.clipRect.bottom <= cmd.clipRect.top) continue;

			D3D11_RECT scissor = { (LONG)cmd.clipRect.left, (LONG)cmd.clipRect.top, (LONG)cmd.clipRect.right, (LONG)cmd.clipRect.bottom };
			ctx->RSSetScissorRects(1, &scissor);

			ID3D11ShaderResourceView* srv = nullptr;
			if (cmd.textureID != 0 && g_state.textureMap.find(cmd.textureID) != g_state.textureMap.end()) {
				srv = g_state.textureMap[cmd.textureID];
			}
			if (!srv) srv = g_state.fontSrv.Get();

			ctx->PSSetShaderResources(0, 1, &srv);
			ctx->DrawIndexed(cmd.indexCount, cmd.startIndex, cmd.vertexOffset);
		}
	}

	void ImplDX11_uploadFontAtlas(const uint8_t* pixels, uint32_t width, uint32_t height) {

		std::vector<uint8_t> rgbaPixels(width * height * 4);
		for (uint32_t i = 0; i < width * height; i++) {
			rgbaPixels[i * 4 + 0] = 255;
			rgbaPixels[i * 4 + 1] = 255;
			rgbaPixels[i * 4 + 2] = 255;
			rgbaPixels[i * 4 + 3] = pixels[i];
		}

		D3D11_TEXTURE2D_DESC desc = {};
		desc.Width = width;
		desc.Height = height;
		desc.MipLevels = 1;
		desc.ArraySize = 1;
		desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		desc.SampleDesc.Count = 1;
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

		D3D11_SUBRESOURCE_DATA initData = {};
		initData.pSysMem = rgbaPixels.data();
		initData.SysMemPitch = width * 4;

		g_state.device->CreateTexture2D(&desc, &initData, &g_state.fontTexture);
		g_state.device->CreateShaderResourceView(g_state.fontTexture.Get(), nullptr, &g_state.fontSrv);
	}

	TextureID ImplDX11_registerTexture(ID3D11ShaderResourceView* srv) {
		if (!srv) return 0;
		TextureID id = g_state.nextAllocatedTextureID++;
		g_state.textureMap[id] = srv;
		return id;
	}

	void ImplDX11_updateTexture(TextureID id, ID3D11ShaderResourceView* srv) {
		if (id != 0 && srv) {
			g_state.textureMap[id] = srv;
		}
	}

}
