#pragma once

#include "AxionEngine/Source/core/Math.h"
#include "AxionEngine/Source/render/Camera.h"
#include "AxionEngine/Source/render/Mesh.h"
#include "AxionEngine/Source/render/Material.h"
#include "AxionEngine/Source/render/Buffers.h"

namespace Axion {

	struct alignas(16) ObjectBuffer {
		DirectX::XMFLOAT4 color;
		DirectX::XMMATRIX modelMatrix;
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

		static void beginScene(const Camera& cam);
		static void beginScene(const Mat4& projection, const Mat4& transform);
		static void endScene();

		static void setClearColor(const Vec4& color);
		static void clear();

		static void drawMesh(const Mat4& transform, Ref<Mesh>& mesh, Ref<Material>& material, Ref<ConstantBuffer>& uploadBuffer);

	};

}
