#include "axpch.h"
#include "Renderer2D.h"

#include "AxionEngine/Source/core/AssetManager.h"
#include "AxionEngine/Source/render/Renderer.h"
#include "AxionEngine/Source/render/RenderCommand.h"
#include "AxionEngine/Source/render/Texture.h"

namespace Axion {

	Ref<VertexBuffer> Renderer2D::s_vertexBuffer = nullptr;
	Ref<IndexBuffer> Renderer2D::s_indexBuffer = nullptr;
	Ref<ConstantBuffer> Renderer2D::s_constantBuffer = nullptr;
	Ref<Material> Renderer2D::s_material = nullptr;
	bool Renderer2D::s_done = false;
	bool Renderer2D::s_initialized = false;

	void Renderer2D::initialize() {

		std::vector<Vertex> vertices = {
			{ -0.5f,  0.5f, 0, 0,0,1,  0,0 }, // TL  (0)
			{  0.5f,  0.5f, 0, 0,0,1,  1,0 }, // TR  (1)
			{  0.5f, -0.5f, 0, 0,0,1,  1,1 }, // BR  (2)
			{ -0.5f, -0.5f, 0, 0,0,1,  0,1 }, // BL  (3)
		};
		
		std::vector<uint32_t> indices = {
			0, 1, 2,  // First triangle
			0, 2, 3   // Second triangle
		};
		
		s_vertexBuffer = VertexBuffer::create(vertices);
		s_indexBuffer = IndexBuffer::create(indices);
		s_constantBuffer = ConstantBuffer::create(sizeof(ObjectBuffer));

		s_initialized = true;
	}

	void Renderer2D::shutdown() {
		s_vertexBuffer->release();
		s_indexBuffer->release();
		s_constantBuffer->release();
		s_material->release();

		s_initialized = false;
	}

	void Renderer2D::onEvent(Event& e) {
		if (e.getEventType() == EventType::ProjectChanged) {
			AssetHandle<Shader> shaderHandle = AssetManager::load<Shader>(AssetManager::getAbsolute("shaders/Default2DShader.axshader"));
			s_material = Material::create("Default2DMat", { 1.0f, 0.0f, 0.0f, 1.0f }, shaderHandle);
			s_done = true;
		}
	}

	void Renderer2D::beginScene(const Camera& camera) {
		Renderer::beginScene(camera);
	}

	void Renderer2D::endScene() {
		// nothing special - no batching
	}

	void Renderer2D::drawQuad(const Vec2& position, const Vec2& size, const Vec4& color) {
		if (!s_initialized) return;
		if (!s_done) return;

		s_material->use();
		Mat4 transform = Mat4::TRS(
			Vec3(position.x, position.y, 0),
			Vec3(0.0f, 0.0f, 0.0f),
			Vec3(size.x, size.y, 1.0f)
		);

		ObjectBuffer buffer;
		buffer.color = s_material->getColor().toFloat4();
		buffer.modelMatrix = transform.transposed().toXM();

		s_constantBuffer->update(&buffer, sizeof(ObjectBuffer));
		s_constantBuffer->bind(1);

		s_vertexBuffer->bind();
		s_indexBuffer->bind();
		RenderCommand::drawIndexed(s_vertexBuffer, s_indexBuffer);
	}

}
