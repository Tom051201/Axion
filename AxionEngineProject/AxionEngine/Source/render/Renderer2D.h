#pragma once

#include "AxionEngine/Source/core/Core.h"
#include "AxionEngine/Source/render/Camera.h"
#include "AxionEngine/Source/render/Mesh.h"
#include "AxionEngine/Source/render/Buffers.h"
#include "AxionEngine/Source/render/Material.h"
#include "AxionEngine/Source/render/Texture.h"
#include "AxionEngine/Source/events/Event.h"

namespace Axion {

	class Renderer2D {
	public:

		struct Statistics {
			uint32_t drawCalls = 0;
			uint32_t quadCount = 0;
			uint32_t getTotalVertexCount() { return quadCount * 4; }
			uint32_t getTotalIndexCount() { return quadCount * 6; }
		};

		Renderer2D() = delete;

		static void initialize();
		static void shutdown();

		static void onEvent(Event& e);

		static void beginScene(const Camera& camera);
		static void endScene();

		static void drawQuad(const Vec2& position, const Vec2& size, const Vec4& color);
		static void drawQuad(const Vec3& position, const Vec2& size, const Vec4& color);

		static void drawQuad(const Vec2& position, const Vec2& size, float rotation, const Vec4& color);
		static void drawQuad(const Vec3& position, const Vec2& size, float rotation, const Vec4& color);

		static void drawQuad(const Vec2& position, const Vec2& size, const Ref<Texture2D>& texture, const Vec4& tint = Vec4::one());
		static void drawQuad(const Vec3& position, const Vec2& size, float rotation, const Ref<Texture2D>& texture, const Vec4& tint = Vec4::one());

		static Statistics getStats();

	private:

		static void flush();
		static void startBatch();
		static void nextBatch();
		static void resetStats();

	};

}
