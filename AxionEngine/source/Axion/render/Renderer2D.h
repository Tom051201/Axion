#pragma once

#include "Axion/core/Core.h"
#include "Axion/render/Buffers.h"
#include "Axion/render/Texture.h"

#include "Axion/core/Math.h"
#include "Axion/render/OrthographicCamera.h"

namespace Axion {

	class Renderer2D {
	public:

		static void initialize();
		static void shutdown();

		static void beginScene(const OrthographicCamera& cam);
		static void endScene();

		static void setClearColor(const Vec4& color);
		static const Vec4& getClearColor();
		static void clear();
		static void present();

		static void drawQuad(const Vec3& position, const Vec2& dim, const Vec4& color);
		static void drawQuad(const Vec3& position, const Vec2& dim, const Vec4& color, Ref<ConstantBuffer>& uploadBuffer);
		static void drawQuad(const Mat4& transform, const Vec4& color, Ref<ConstantBuffer>& uploadBuffer);

		static void drawTexture(const Vec3& position, const Vec2& dim, Ref<Texture2D>& texture, Ref<ConstantBuffer>& uploadBuffer);

	};

}
