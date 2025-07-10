#include "axpch.h"
#include "Renderer2D.h"

#include "AxionEngine/Source/core/Math.h"
#include "AxionEngine/Source/render/Renderer.h"
#include "AxionEngine/Source/render/RenderCommand.h"
#include "AxionEngine/Source/render/Mesh.h"
#include "AxionEngine/Source/render/Shader.h"

namespace Axion {

	struct alignas(16) SceneData {
		DirectX::XMMATRIX viewProjection;
	};

	struct RendererData {
		Ref<Mesh> quadMesh;
		Ref<Shader> quadShader;
	};

	static SceneData* s_sceneData;
	static Ref<ConstantBuffer> s_sceneUploadBuffer;

	static RendererData* s_rendererData;

	void Renderer2D::initialize() {
		s_sceneData = new SceneData();
		s_sceneUploadBuffer = ConstantBuffer::create(sizeof(SceneData));

		s_rendererData = new RendererData();

		s_rendererData->quadShader = Axion::Shader::create("quadShader2D");
		s_rendererData->quadShader->compileFromFile("AxionStudio/Assets/shaders/ColorShader.hlsl");

		std::vector<Axion::Vertex> vertices = {
			Axion::Vertex(-0.5f, -0.5f, 0.0f,	1.0f, 1.0f, 0.0f, 1.0f,		0.0f, 0.0f),
			Axion::Vertex(0.5f, -0.5f, 0.0f,	0.0f, 0.0f, 1.0f, 1.0f,		1.0f, 0.0f),
			Axion::Vertex(0.5f,  0.5f, 0.0f,	0.0f, 1.0f, 0.0f, 1.0f,		1.0f, 1.0f),
			Axion::Vertex(-0.5f,  0.5f, 0.0f,	1.0f, 0.0f, 0.0f, 1.0f,		0.0f, 1.0f)
		};
		std::vector<uint32_t> indices = { 0, 2, 1,	2, 0, 3 };
		s_rendererData->quadMesh = Axion::Mesh::create(vertices, indices);
	}

	void Renderer2D::shutdown() {
		delete s_sceneData;
		s_sceneUploadBuffer->release();
		delete s_rendererData;
	}

	void Renderer2D::beginScene(const Camera& camera, const Mat4& transform) {
		Mat4 viewProj = camera.getProjectionMatrix() * (transform.inverse());
		s_rendererData->quadShader->bind();
		s_sceneData->viewProjection = DirectX::XMMatrixTranspose(viewProj.toXM());
		s_sceneUploadBuffer->update(s_sceneData, sizeof(SceneData));
		s_sceneUploadBuffer->bind(0);
	}

	void Renderer2D::beginScene(const Camera& camera) {
		s_rendererData->quadShader->bind();
		s_sceneData->viewProjection = DirectX::XMMatrixTranspose(camera.getViewProjectionMatrix().toXM());
		s_sceneUploadBuffer->update(s_sceneData, sizeof(SceneData));
		s_sceneUploadBuffer->bind(0);
	}

	void Renderer2D::beginScene(const OrthographicCamera& camera) {
		s_rendererData->quadShader->bind();
		s_sceneData->viewProjection = DirectX::XMMatrixTranspose(camera.getViewProjectionMatrix().toXM());
		s_sceneUploadBuffer->update(s_sceneData, sizeof(SceneData));
		s_sceneUploadBuffer->bind(0);
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

	void Renderer2D::drawQuad(const Vec3& position, const Vec2& dim, const Vec4& color, Ref<ConstantBuffer>& uploadBuffer) {
		s_rendererData->quadShader->bind();

		ObjectBuffer buffer;
		buffer.color = color.toFloat4();

		Mat4 model = Mat4::translation(position) * Mat4::scale(Vec3(dim.x, dim.y, 1.0f));
		buffer.modelMatrix = model.transposed().toXM();

		uploadBuffer->update(&buffer, sizeof(ObjectBuffer));
		uploadBuffer->bind(1);

		s_rendererData->quadMesh->render();
		RenderCommand::drawIndexed(s_rendererData->quadMesh->getVertexBuffer(), s_rendererData->quadMesh->getIndexBuffer());
	}

	void Renderer2D::drawQuad(const Mat4& transform, const Vec4& color, Ref<ConstantBuffer>& uploadBuffer) {
		s_rendererData->quadShader->bind();

		ObjectBuffer buffer;
		buffer.color = color.toFloat4();

		buffer.modelMatrix = transform.transposed().toXM();

		uploadBuffer->update(&buffer, sizeof(ObjectBuffer));
		uploadBuffer->bind(1);

		s_rendererData->quadMesh->render();
		RenderCommand::drawIndexed(s_rendererData->quadMesh->getVertexBuffer(), s_rendererData->quadMesh->getIndexBuffer());
	}

	void Renderer2D::drawTexture(const Vec3& position, const Vec2& dim, Ref<Texture2D>& texture, Ref<ConstantBuffer>& uploadBuffer) {
		s_rendererData->quadShader->bind();

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
