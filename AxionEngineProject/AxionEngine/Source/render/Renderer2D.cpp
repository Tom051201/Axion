#include "axpch.h"
#include "Renderer2D.h"

#include "AxionEngine/Source/core/Math.h"
#include "AxionEngine/Source/render/Renderer.h"
#include "AxionEngine/Source/render/RenderCommand.h"
#include "AxionEngine/Source/render/Mesh.h"
#include "AxionEngine/Source/render/Shader.h"

namespace Axion {

	struct RendererData {
		Ref<Mesh> quadMesh;
	};

	static RendererData* s_rendererData;

	void Renderer2D::initialize() {
		s_rendererData = new RendererData();

		std::vector<Vertex> vertices = {
			Vertex(-0.5f, -0.5f, 0.0f,		0.0f, 0.0f, 1.0f,		0.0f, 0.0f),
			Vertex( 0.5f, -0.5f, 0.0f,		0.0f, 0.0f, 1.0f,		1.0f, 0.0f),
			Vertex( 0.5f,  0.5f, 0.0f,		0.0f, 0.0f, 1.0f,		1.0f, 1.0f),
			Vertex(-0.5f,  0.5f, 0.0f,		0.0f, 0.0f, 1.0f,		0.0f, 1.0f)
		};
		std::vector<uint32_t> indices = { 0, 2, 1,	2, 0, 3 };
		s_rendererData->quadMesh = Mesh::create({}, vertices, indices); // TODO : fix empty handle here
	}

	void Renderer2D::shutdown() {
		delete s_rendererData;
	}

	void Renderer2D::beginScene(const Mat4& projection, const Mat4& transform) {
		Renderer::beginScene(projection, transform);
	}

	void Renderer2D::beginScene(const Camera& camera) {
		Renderer::beginScene(camera);
	}

	void Renderer2D::endScene() {
		// Does nothing for now
	}

	void Renderer2D::setClearColor(const Vec4& color) {
		RenderCommand::setClearColor(color);
	}

	void Renderer2D::clear() {
		RenderCommand::clear();
	}

	void Renderer2D::drawQuad(const Mat4& transform, Ref<Material>& material, Ref<ConstantBuffer>& uploadBuffer) {
		material->use();

		ObjectBuffer buffer;
		buffer.color = material->getColor().toFloat4();
		buffer.modelMatrix = transform.transposed().toXM();

		uploadBuffer->update(&buffer, sizeof(ObjectBuffer));
		uploadBuffer->bind(1);

		s_rendererData->quadMesh->render();
		RenderCommand::drawIndexed(s_rendererData->quadMesh->getVertexBuffer(), s_rendererData->quadMesh->getIndexBuffer());
	}

	void Renderer2D::drawTexture(const Vec3& position, const Vec2& dim, Ref<Texture2D>& texture, Ref<ConstantBuffer>& uploadBuffer) {
		// TODO: rewrite this
		//s_rendererData->quadShader->bind();

		ObjectBuffer buffer;
		buffer.color = Vec4::one().toFloat4();

		Mat4 model = Mat4::translation(position) * Mat4::scale(Vec3(dim.x, dim.y, 1.0f));
		buffer.modelMatrix = model.transposed().toXM();
		
		uploadBuffer->update(&buffer, sizeof(ObjectBuffer));
		uploadBuffer->bind(1);

		s_rendererData->quadMesh->render();
		texture->bind();
		
		RenderCommand::drawIndexed(s_rendererData->quadMesh->getVertexBuffer(), s_rendererData->quadMesh->getIndexBuffer());
	}

}
