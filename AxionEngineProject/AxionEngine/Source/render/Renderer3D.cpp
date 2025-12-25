#include "axpch.h"
#include "Renderer3D.h"

#include "AxionEngine/Source/render/Renderer.h"
#include "AxionEngine/Source/render/RenderCommand.h"

namespace Axion {

	void Renderer3D::initialize() {
		AX_CORE_LOG_TRACE("Renderer3D initialized");
	}

	void Renderer3D::shutdown() {
		AX_CORE_LOG_TRACE("Renderer3D shutdown");
	}

	void Renderer3D::beginScene(const Camera& cam) {
		Renderer::beginScene(cam);
	}

	void Renderer3D::beginScene(const Mat4& projection, const Mat4& transform) {
		Renderer::beginScene(projection, transform);
	}

	void Renderer3D::endScene() {
		// does nothing for now
	}

	void Renderer3D::setClearColor(const Vec4& color) {
		RenderCommand::setClearColor(color);
	}

	void Renderer3D::clear() {
		RenderCommand::clear();
	}

	void Renderer3D::drawMesh(const Mat4& transform, Ref<Mesh>& mesh, Ref<Material>& material, Ref<ConstantBuffer>& uploadBuffer) {
		material->use();

		ObjectBuffer buffer;
		buffer.color = material->getColor().toFloat4();
		buffer.modelMatrix = transform.transposed().toXM();

		uploadBuffer->update(&buffer, sizeof(ObjectBuffer));
		uploadBuffer->bind(1);

		mesh->render();
		RenderCommand::drawIndexed(mesh->getVertexBuffer(), mesh->getIndexBuffer());
	}

}
