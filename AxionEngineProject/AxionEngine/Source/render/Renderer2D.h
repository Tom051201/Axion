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

		Renderer2D() = delete;

		static void initialize();
		static void shutdown();

		static void onEvent(Event& e);

		static void beginScene(const Camera& camera);
		static void endScene();

		static void drawQuad(const Vec2& position, const Vec2& size, const Vec4& color);

	private:

		static Ref<VertexBuffer> s_vertexBuffer;
		static Ref<IndexBuffer> s_indexBuffer;

		static Ref<Material> s_material;
		static Ref<ConstantBuffer> s_constantBuffer;

		static bool s_done;

		static bool s_initialized;

	};

}
