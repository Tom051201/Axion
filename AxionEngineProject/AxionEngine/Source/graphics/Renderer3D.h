#pragma once

#include <cstdint>
#include <vector>
#include <DirectXMath.h>

#include "AxionEngine/Source/core/Math.h"
#include "AxionEngine/Source/graphics/Camera.h"
#include "AxionEngine/Source/graphics/Mesh.h"
#include "AxionEngine/Source/graphics/SkeletalMesh.h"
#include "AxionEngine/Source/graphics/Material.h"
#include "AxionEngine/Source/graphics/Buffers.h"
#include "AxionEngine/Source/graphics/Renderer.h"

namespace Axion {

	class RenderContext;

	struct alignas(16) ObjectBuffer {
		DirectX::XMFLOAT4 color; // TODO: remove this and remove from shaders
		DirectX::XMMATRIX modelMatrix;
		int entityID = -1;
		DirectX::XMFLOAT3 padding;
	};

	struct alignas(16) SkeletalObjectBuffer {
		DirectX::XMFLOAT4X4 modelMatrix;
		DirectX::XMFLOAT4 color;
		DirectX::XMFLOAT4X4 boneTransforms[100];
		int entityID = -1;
		DirectX::XMFLOAT3 padding;
	};

	class Renderer3D {
	public:

		Renderer3D() = delete;
		~Renderer3D() = delete;
		Renderer3D(const Renderer3D&) = delete;
		Renderer3D(Renderer3D&&) = delete;
		Renderer3D& operator=(const Renderer3D&) = delete;
		Renderer3D& operator=(Renderer3D&&) = delete;

		static void initialize();
		static void shutdown();

		static void beginScene(const Camera& cam, const LightingData& lightingData);
		static void beginScene(const Mat4& projection, const Mat4& transform);
		static void endScene();
		static void beginFrame();

		static void setClearColor(const Vec4& color);
		static void clear(RenderContext* renderContext);

		static void drawMesh(RenderContext* renderContext, const Mat4& transform, Ref<Mesh>& mesh, uint32_t submeshIndex, Ref<Material>& material, Ref<ConstantBuffer>& uploadBuffer);
		static void drawMeshInstanced(RenderContext* renderContext, Ref<Mesh>& mesh, uint32_t submeshIndex, Ref<Material>& material, const ObjectBuffer* instanceData, uint32_t instanceCount);

		static void drawSkeletalMeshInstanced(RenderContext* renderContext, Ref<SkeletalMesh>& mesh, uint32_t submeshIndex, Ref<Material>& material, const SkeletalObjectBuffer* instanceData, uint32_t instanceCount);

		static void drawMeshInstancedShadow(RenderContext* renderContext, Ref<Mesh>& mesh, uint32_t submeshIndex, const ObjectBuffer* instanceData, uint32_t instanceCount);
		static void drawSkeletalMeshInstancedShadow(RenderContext* renderContext, Ref<SkeletalMesh>& mesh, uint32_t submeshIndex, const SkeletalObjectBuffer* instanceData, uint32_t instanceCount);

	};

}
