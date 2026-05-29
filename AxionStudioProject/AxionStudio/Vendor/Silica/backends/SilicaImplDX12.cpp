#include "SilicaImplDX12.h"

#include <d3dcompiler.h>
#include <wrl/client.h>
#include <stdexcept>

#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "d3dcompiler.lib")

// CHANGED TO MATCH AXION
//#include "d3d12/d3dx12/d3dx12.h"
#include "AxionEngine/Vendor/d3d12/d3dx12/d3dx12.h"

using Microsoft::WRL::ComPtr;

namespace Silica {

	struct BackendStateDX12 {
		ID3D12Device* device = nullptr;
		UINT numFramesInFlight = 0;
		UINT frameIndex = 0;

		ComPtr<ID3D12RootSignature> rootSignature;
		ComPtr<ID3D12PipelineState> pipelineState;

		ComPtr<ID3D12Resource> vertexBuffer;
		ComPtr<ID3D12Resource> indexBuffer;

		Vertex* vertexData = nullptr;
		uint32_t* indexData = nullptr;

		uint32_t maxVertices = 10000;
		uint32_t maxIndices = maxVertices * 3;

		ComPtr<ID3D12DescriptorHeap> srvHeap;
		ComPtr<ID3D12Resource> fontTexture;
		ComPtr<ID3D12Resource> fontUploadHeap;

		UINT srvDescriptorSize = 0;
		uint32_t nextAllocatedTextureID = 0;
	};

	static BackendStateDX12 g_state;



	// ----- Helper To Compile Shader -----
	static ComPtr<ID3DBlob> compileShader(const char* shaderCode, const char* entryPoint, const char* target) {
		ComPtr<ID3DBlob> shaderBlob;
		ComPtr<ID3DBlob> errorBlob;
		UINT flags = D3DCOMPILE_ENABLE_STRICTNESS;

		HRESULT hr = D3DCompile(
			shaderCode,
			strlen(shaderCode),
			nullptr, nullptr, nullptr,
			entryPoint,
			target,
			flags,
			0,
			&shaderBlob,
			&errorBlob
		);

		if (FAILED(hr)) {
			if (errorBlob) {
				OutputDebugStringA((char*)errorBlob->GetBufferPointer());
			}
			throw std::runtime_error("Failed to compile Silica UI Shader!");
		}

		return shaderBlob;
	}



	// ----- Init Silica DX12 Backend -----
	bool ImplDX12_init(ID3D12Device* device, UINT numFramesInFlight, DXGI_FORMAT rtvFormat) {
		g_state.device = device;
		g_state.numFramesInFlight = numFramesInFlight;
		g_state.frameIndex = 0;


		// -- Create Root Signature --
		CD3DX12_DESCRIPTOR_RANGE srvRange;
		srvRange.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0); // t0
		CD3DX12_ROOT_PARAMETER rootParameters[2];
		rootParameters[0].InitAsConstants(16, 0, 0, D3D12_SHADER_VISIBILITY_VERTEX);
		rootParameters[1].InitAsDescriptorTable(1, &srvRange, D3D12_SHADER_VISIBILITY_PIXEL);
		CD3DX12_STATIC_SAMPLER_DESC sampler = {};
		sampler.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
		sampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
		sampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
		sampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
		sampler.ShaderRegister = 0; // s0
		D3D12_ROOT_SIGNATURE_DESC rootSigDesc = {};
		rootSigDesc.NumParameters = _countof(rootParameters);
		rootSigDesc.pParameters = rootParameters;
		rootSigDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
		rootSigDesc.NumStaticSamplers = 1;
		rootSigDesc.pStaticSamplers = &sampler;

		ComPtr<ID3DBlob> signatureBlob;
		ComPtr<ID3DBlob> errorBlob;
		D3D12SerializeRootSignature(&rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signatureBlob, &errorBlob);
		device->CreateRootSignature(0, signatureBlob->GetBufferPointer(), signatureBlob->GetBufferSize(), IID_PPV_ARGS(&g_state.rootSignature));


		// -- Create HLSL Shader Code --
		const char* shaderSource = R"(
			cbuffer RootConstants : register(b0) {
				float4x4 ProjectionMatrix;
			};

			struct VS_INPUT {
				float2 pos : POSITION;
				float2 uv  : TEXCOORD0;
				uint color : COLOR0;
			};

			Texture2D fontTex : register(t0);
			SamplerState fontSampler : register(s0);

			struct PS_INPUT {
				float4 position : SV_POSITION;
				float2 uv : TEXCOORD;
				float4 color : COLOR;
			};

			PS_INPUT VSMain(VS_INPUT input) {
				PS_INPUT output;
				output.position = mul(ProjectionMatrix, float4(input.pos.xy, 0.0f, 1.0f));
				output.uv = input.uv;
				output.color = float4((input.color & 0xFF) / 255.0f,
									((input.color >> 8) & 0xFF) / 255.0f,
									((input.color >> 16) & 0xFF) / 255.0f,
									((input.color >> 24) & 0xFF) / 255.0f);
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
				} 
				else {
					float fontAlpha = fontTex.Sample(fontSampler, input.uv).r;
					finalColor.a *= fontAlpha;
				}

				return finalColor;
			}
		)";

		ComPtr<ID3DBlob> vsBlob = compileShader(shaderSource, "VSMain", "vs_5_0");
		ComPtr<ID3DBlob> psBlob = compileShader(shaderSource, "PSMain", "ps_5_0");


		// -- Input Layout --
		D3D12_INPUT_ELEMENT_DESC inputElementDescs[] = {
			{ "POSITION",	0,	DXGI_FORMAT_R32G32_FLOAT,	0,	offsetof(Vertex, position),		D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,		0 },
			{ "TEXCOORD",	0,	DXGI_FORMAT_R32G32_FLOAT,	0,	offsetof(Vertex, uv),			D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,		0 },
			{ "COLOR",		0,	DXGI_FORMAT_R32_UINT,		0,	offsetof(Vertex, color),		D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,		0 },
		};


		// -- Create Pipeline State Object (PSO) --
		D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
		psoDesc.InputLayout = { inputElementDescs, _countof(inputElementDescs) };
		psoDesc.pRootSignature = g_state.rootSignature.Get();
		psoDesc.VS = { vsBlob->GetBufferPointer(), vsBlob->GetBufferSize() };
		psoDesc.PS = { psBlob->GetBufferPointer(), psBlob->GetBufferSize() };
		psoDesc.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
		psoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
		psoDesc.BlendState.RenderTarget[0].BlendEnable = TRUE;
		psoDesc.BlendState.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
		psoDesc.BlendState.RenderTarget[0].DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
		psoDesc.BlendState.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
		psoDesc.BlendState.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
		psoDesc.BlendState.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_INV_SRC_ALPHA;
		psoDesc.BlendState.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
		psoDesc.BlendState.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
		psoDesc.SampleMask = UINT_MAX;
		psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		psoDesc.NumRenderTargets = 1;
		psoDesc.RTVFormats[0] = rtvFormat;
		psoDesc.SampleDesc.Count = 1;
		psoDesc.DepthStencilState.DepthEnable = FALSE;

		device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&g_state.pipelineState));


		// -- Create Dynamic Upload Buffers --
		uint32_t vbSize = g_state.maxVertices * sizeof(Vertex) * numFramesInFlight;
		uint32_t ibSize = g_state.maxIndices * sizeof(uint32_t) * numFramesInFlight;

		D3D12_HEAP_PROPERTIES uploadHeap = { D3D12_HEAP_TYPE_UPLOAD, D3D12_CPU_PAGE_PROPERTY_UNKNOWN, D3D12_MEMORY_POOL_UNKNOWN, 1, 1 };
		D3D12_RESOURCE_DESC vbDesc = { D3D12_RESOURCE_DIMENSION_BUFFER, 0, vbSize, 1, 1, 1, DXGI_FORMAT_UNKNOWN, 1, 0, D3D12_TEXTURE_LAYOUT_ROW_MAJOR, D3D12_RESOURCE_FLAG_NONE };
		D3D12_RESOURCE_DESC ibDesc = { D3D12_RESOURCE_DIMENSION_BUFFER, 0, ibSize, 1, 1, 1, DXGI_FORMAT_UNKNOWN, 1, 0, D3D12_TEXTURE_LAYOUT_ROW_MAJOR, D3D12_RESOURCE_FLAG_NONE };

		device->CreateCommittedResource(&uploadHeap, D3D12_HEAP_FLAG_NONE, &vbDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&g_state.vertexBuffer));
		device->CreateCommittedResource(&uploadHeap, D3D12_HEAP_FLAG_NONE, &ibDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&g_state.indexBuffer));

		g_state.vertexBuffer->Map(0, nullptr, reinterpret_cast<void**>(&g_state.vertexData));
		g_state.indexBuffer->Map(0, nullptr, reinterpret_cast<void**>(&g_state.indexData));


		return true;
	}



	// ----- Shutdown Silica DX12 Backend -----
	void ImplDX12_shutdown() {
		if (g_state.vertexBuffer) g_state.vertexBuffer->Unmap(0, nullptr);
		if (g_state.indexBuffer) g_state.indexBuffer->Unmap(0, nullptr);

		g_state.fontTexture.Reset();
		g_state.fontUploadHeap.Reset();
		g_state.srvHeap.Reset();

		g_state.vertexBuffer.Reset();
		g_state.indexBuffer.Reset();
		g_state.pipelineState.Reset();
		g_state.rootSignature.Reset();
		g_state.device = nullptr;
	}



	// ----- Advance Frame Index --
	void ImplDX12_newFrame() {
		g_state.frameIndex = (g_state.frameIndex + 1) % g_state.numFramesInFlight;
	}



	// ----- Do The Rendering -----
	void ImplDX12_renderDrawData(const DrawList* drawData, ID3D12GraphicsCommandList* commandList, float screenWidth, float screenHeight) {
		if (!drawData || drawData->vertices.empty() || drawData->indices.empty()) return;

		size_t vertexCount = drawData->vertices.size();
		size_t indexCount = drawData->indices.size();

		// -- Calculate Offset for this Frame --
		uint32_t vbOffsetBytes = g_state.frameIndex * g_state.maxVertices * sizeof(Vertex);
		uint32_t ibOffsetBytes = g_state.frameIndex * g_state.maxIndices * sizeof(uint32_t);

		// -- Copy Data --
		memcpy((uint8_t*)g_state.vertexData + vbOffsetBytes, drawData->vertices.data(), vertexCount * sizeof(Vertex));
		memcpy((uint8_t*)g_state.indexData + ibOffsetBytes, drawData->indices.data(), indexCount * sizeof(uint32_t));

		// -- Set Pipeline and Root Signature --
		commandList->SetPipelineState(g_state.pipelineState.Get());
		commandList->SetGraphicsRootSignature(g_state.rootSignature.Get());

		// -- Calculate Orthographic Projection --
		float L = 0.0f;
		float R = screenWidth;
		float T = 0.0f;
		float B = screenHeight;
		float mvp[4][4] = {
			{ 2.0f / (R - L),		0.0f,						0.0f,			0.0f },
			{ 0.0f,					2.0f / (T - B),				0.0f,			0.0f },
			{ 0.0f,					0.0f,						0.5f,			0.0f },
			{ (L + R) / (L - R),	(T + B) / (B - T),			0.5f,			1.0f },
		};

		commandList->SetGraphicsRoot32BitConstants(0, 16, &mvp[0][0], 0);

		// -- Bind Vertex And Index Buffers --
		D3D12_VERTEX_BUFFER_VIEW vbv;
		vbv.BufferLocation = g_state.vertexBuffer->GetGPUVirtualAddress() + vbOffsetBytes;
		vbv.SizeInBytes = (UINT)(vertexCount * sizeof(Vertex));
		vbv.StrideInBytes = sizeof(Vertex);

		D3D12_INDEX_BUFFER_VIEW ibv;
		ibv.BufferLocation = g_state.indexBuffer->GetGPUVirtualAddress() + ibOffsetBytes;
		ibv.SizeInBytes = (UINT)(indexCount * sizeof(uint32_t));
		ibv.Format = DXGI_FORMAT_R32_UINT;

		commandList->IASetVertexBuffers(0, 1, &vbv);
		commandList->IASetIndexBuffer(&ibv);
		commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		ID3D12DescriptorHeap* descriptorHeaps[] = { g_state.srvHeap.Get() };
		commandList->SetDescriptorHeaps(1, descriptorHeaps);

		// -- Draw Commands --
		for (const auto& cmd : drawData->commands) {
			if (cmd.indexCount == 0) continue;
			if (cmd.clipRect.right <= cmd.clipRect.left || cmd.clipRect.bottom <= cmd.clipRect.top) {
				continue;
			}

			CD3DX12_GPU_DESCRIPTOR_HANDLE gpuHandle(
				g_state.srvHeap->GetGPUDescriptorHandleForHeapStart(),
				cmd.textureID,
				g_state.srvDescriptorSize
			);
			commandList->SetGraphicsRootDescriptorTable(1, gpuHandle);

			D3D12_RECT scissor;
			scissor.left = static_cast<LONG>(cmd.clipRect.left);
			scissor.top = static_cast<LONG>(cmd.clipRect.top);
			scissor.right = static_cast<LONG>(cmd.clipRect.right);
			scissor.bottom = static_cast<LONG>(cmd.clipRect.bottom);
			commandList->RSSetScissorRects(1, &scissor);

			commandList->DrawIndexedInstanced(
				cmd.indexCount,
				1,
				cmd.startIndex,
				cmd.vertexOffset,
				0
			);
		}
	}

	void ImplDX12_uploadFontAtlas(ID3D12GraphicsCommandList* cmdList, const uint8_t* pixels, uint32_t width, uint32_t height) {
		// -- Create SRV Descriptor Heap --
		D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc = {};
		srvHeapDesc.NumDescriptors = 1024;
		srvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		srvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
		g_state.device->CreateDescriptorHeap(&srvHeapDesc, IID_PPV_ARGS(&g_state.srvHeap));

		g_state.srvDescriptorSize = g_state.device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		g_state.nextAllocatedTextureID = 1;

		// -- Create the Texture Resource (R8_UNORM) --
		CD3DX12_RESOURCE_DESC texDesc = CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_R8_UNORM, width, height, 1, 1);
		CD3DX12_HEAP_PROPERTIES defaultHeap(D3D12_HEAP_TYPE_DEFAULT);
		g_state.device->CreateCommittedResource(
			&defaultHeap,
			D3D12_HEAP_FLAG_NONE,
			&texDesc,
			D3D12_RESOURCE_STATE_COPY_DEST,
			nullptr,
			IID_PPV_ARGS(&g_state.fontTexture)
		);

		// -- Create The Upload Heap --
		const UINT uploadPitch = (width + D3D12_TEXTURE_DATA_PITCH_ALIGNMENT - 1u) & ~(D3D12_TEXTURE_DATA_PITCH_ALIGNMENT - 1u);
		const UINT uploadSize = height * uploadPitch;
		CD3DX12_HEAP_PROPERTIES uploadHeap(D3D12_HEAP_TYPE_UPLOAD);
		CD3DX12_RESOURCE_DESC bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(uploadSize);
		g_state.device->CreateCommittedResource(
			&uploadHeap,
			D3D12_HEAP_FLAG_NONE,
			&bufferDesc,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&g_state.fontUploadHeap)
		);

		// -- Copy CPU Data to Upload Heap --
		void* mapped = nullptr;
		g_state.fontUploadHeap->Map(0, nullptr, &mapped);
		for (uint32_t y = 0; y < height; y++) {
			memcpy((void*)((uintptr_t)mapped + y * uploadPitch), pixels + y * width, width);
		}
		g_state.fontUploadHeap->Unmap(0, nullptr);

		// -- Copy Upload Heap to Texture --
		D3D12_TEXTURE_COPY_LOCATION dst = {};
		dst.pResource = g_state.fontTexture.Get();
		dst.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
		dst.SubresourceIndex = 0;

		D3D12_PLACED_SUBRESOURCE_FOOTPRINT footprint = {};
		footprint.Footprint.Format = DXGI_FORMAT_R8_UNORM;
		footprint.Footprint.Width = width;
		footprint.Footprint.Height = height;
		footprint.Footprint.Depth = 1;
		footprint.Footprint.RowPitch = uploadPitch;

		D3D12_TEXTURE_COPY_LOCATION src = {};
		src.pResource = g_state.fontUploadHeap.Get();
		src.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
		src.PlacedFootprint = footprint;

		cmdList->CopyTextureRegion(&dst, 0, 0, 0, &src, nullptr);

		// -- Transition Texture to Shader Resource State --
		CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(
			g_state.fontTexture.Get(),
			D3D12_RESOURCE_STATE_COPY_DEST,
			D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE
		);
		cmdList->ResourceBarrier(1, &barrier);

		// -- Create Shader Resource View (SRV) in the Descriptor Heap --
		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.Format = DXGI_FORMAT_R8_UNORM;
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srvDesc.Texture2D.MipLevels = 1;
		g_state.device->CreateShaderResourceView(g_state.fontTexture.Get(), &srvDesc, g_state.srvHeap->GetCPUDescriptorHandleForHeapStart());
	}

	TextureID ImplDX12_registerTexture(ID3D12Resource* textureResource) {
		if (!textureResource || !g_state.srvHeap) return 0;
		if (g_state.nextAllocatedTextureID >= 1024) return 0;

		TextureID newId = g_state.nextAllocatedTextureID++;

		CD3DX12_CPU_DESCRIPTOR_HANDLE cpuHandle(
			g_state.srvHeap->GetCPUDescriptorHandleForHeapStart(),
			newId,
			g_state.srvDescriptorSize
		);

		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srvDesc.Texture2D.MipLevels = 1;

		g_state.device->CreateShaderResourceView(textureResource, &srvDesc, cpuHandle);

		return newId;
	}

}
