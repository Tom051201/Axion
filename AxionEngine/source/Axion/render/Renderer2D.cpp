#include "axpch.h"
#include "Renderer2D.h"

#include "Axion/core/Math.h"
#include "Axion/render/Renderer.h"
#include "Axion/render/RenderCommand.h"
#include "Axion/render/Mesh.h"
#include "Axion/render/Shader.h"

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
	static Ref<ConstantBuffer> s_objectUploadBuffer;

	static RendererData* s_rendererData;

	void Renderer2D::initialize() {
		s_sceneData = new SceneData();
		s_sceneUploadBuffer = ConstantBuffer::create(sizeof(SceneData));
		s_objectUploadBuffer = ConstantBuffer::create(sizeof(ObjectBuffer));

		s_rendererData = new RendererData();

		s_rendererData->quadShader = Axion::Shader::create("quadShader2D");
		s_rendererData->quadShader->compileFromFile("assets/Shader1.hlsl");

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
		s_objectUploadBuffer->release();
		delete s_rendererData;
	}

	void Renderer2D::beginScene(const OrthographicCamera& camera) {
		Renderer::getAPIInstance()->beginScene();

		s_rendererData->quadShader->bind();
		s_sceneData->viewProjection = DirectX::XMMatrixTranspose(camera.getViewProjectionMatrix().toXM());
		s_sceneUploadBuffer->update(s_sceneData, sizeof(SceneData));
		s_sceneUploadBuffer->bind(0);
	}

	void Renderer2D::endScene() {
		Renderer::getAPIInstance()->endScene();
	}

	void Renderer2D::setClearColor(const Vec4& color) {
		RenderCommand::setClearColor(color);
	}

	void Renderer2D::clear() {
		RenderCommand::clear();
	}

	void Renderer2D::present() {
		Renderer::getAPIInstance()->present();
	}

	void Renderer2D::drawQuad(const Vec3& position, const Vec2& dim, const Vec4& color) {
		s_rendererData->quadShader->bind();

		ObjectBuffer buffer;
		buffer.color = color.toFloat4();

		Mat4 model = Mat4::translation(position) * Mat4::scale(Vec3(dim.x, dim.y, 1.0f));
		buffer.modelMatrix = model.transposed().toXM();

		s_objectUploadBuffer->update(&buffer, sizeof(ObjectBuffer));
		s_objectUploadBuffer->bind(1);

		s_rendererData->quadMesh->render();
		RenderCommand::drawIndexed(s_rendererData->quadMesh->getVertexBuffer(), s_rendererData->quadMesh->getIndexBuffer());
	}

}
