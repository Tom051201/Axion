#pragma once

#include "AxionEngine/Source/core/Core.h"
#include "AxionEngine/Source/core/Math.h"
#include "AxionEngine/Source/render/Buffers.h"
#include "AxionEngine/Source/render/Texture.h"
#include "AxionEngine/Source/render/Camera.h"
#include "AxionEngine/Source/render/Material.h"

namespace Axion {

	class Renderer2D {
	public:

		static void initialize();
		static void shutdown();

		static void beginScene(const Camera& cam);
		static void beginScene(const Mat4& projection, const Mat4& transform);
		static void endScene();

		static void setClearColor(const Vec4& color);
		static void clear();

		static void drawQuad(const Mat4& transform, Ref<Material>& material, Ref<ConstantBuffer>& uploadBuffer);
		static void drawTexture(const Vec3& position, const Vec2& dim, Ref<Texture2D>& texture, Ref<ConstantBuffer>& uploadBuffer);

	};

}
