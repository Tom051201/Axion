#include "studiopch.h"
#include "WireframeRenderer.h"

#include "AxionEngine/Source/graphics/Renderer2D.h"

namespace {

	constexpr int WIREFRAME_SEGMENTS = 24;
	std::array<Axion::Vec3, WIREFRAME_SEGMENTS> g_unitCircleXY;
	std::array<Axion::Vec3, WIREFRAME_SEGMENTS> g_unitCircleXZ;
	std::array<Axion::Vec3, WIREFRAME_SEGMENTS> g_unitCircleYZ;

	struct WireframeInit {
		WireframeInit() {
			float step = 360.0f / WIREFRAME_SEGMENTS;
			for (int i = 0; i < WIREFRAME_SEGMENTS; i++) {
				float angle = Axion::Math::toRadians(i * step);
				float c = std::cos(angle);
				float s = std::sin(angle);
				g_unitCircleXY[i] = { c, s, 0.0f };
				g_unitCircleXZ[i] = { c, 0.0f, s };
				g_unitCircleYZ[i] = { 0.0f, c, s };
			}
		}
	} g_wireframeInit;
}

namespace Axion {

	void WireframeRenderer::drawBox(const Mat4& transform, const Vec4& color) {
		Vec3 corners[8] = {
			{ -0.5f, -0.5f, -0.5f }, {  0.5f, -0.5f, -0.5f },
			{  0.5f,  0.5f, -0.5f }, { -0.5f,  0.5f, -0.5f },
			{ -0.5f, -0.5f,  0.5f }, {  0.5f, -0.5f,  0.5f },
			{  0.5f,  0.5f,  0.5f }, { -0.5f,  0.5f,  0.5f }
		};

		for (int i = 0; i < 8; i++) corners[i] = (transform * corners[i]);

		// -- Bottom --
		Renderer2D::drawLine(corners[0], corners[1], color);
		Renderer2D::drawLine(corners[1], corners[2], color);
		Renderer2D::drawLine(corners[2], corners[3], color);
		Renderer2D::drawLine(corners[3], corners[0], color);

		// -- Top --
		Renderer2D::drawLine(corners[4], corners[5], color);
		Renderer2D::drawLine(corners[5], corners[6], color);
		Renderer2D::drawLine(corners[6], corners[7], color);
		Renderer2D::drawLine(corners[7], corners[4], color);

		// -- Sides --
		Renderer2D::drawLine(corners[0], corners[4], color);
		Renderer2D::drawLine(corners[1], corners[5], color);
		Renderer2D::drawLine(corners[2], corners[6], color);
		Renderer2D::drawLine(corners[3], corners[7], color);
	}

	void WireframeRenderer::drawSphere(const Mat4& transform, float radius, const Vec4& color) {
		for (int i = 0; i < WIREFRAME_SEGMENTS; i++) {
			int next = (i + 1) % WIREFRAME_SEGMENTS;

			Vec3 p1_xy = transform * (g_unitCircleXY[i] * radius);
			Vec3 p2_xy = transform * (g_unitCircleXY[next] * radius);
			Renderer2D::drawLine(p1_xy, p2_xy, color);

			Vec3 p1_xz = transform * (g_unitCircleXZ[i] * radius);
			Vec3 p2_xz = transform * (g_unitCircleXZ[next] * radius);
			Renderer2D::drawLine(p1_xz, p2_xz, color);

			Vec3 p1_yz = transform * (g_unitCircleYZ[i] * radius);
			Vec3 p2_yz = transform * (g_unitCircleYZ[next] * radius);
			Renderer2D::drawLine(p1_yz, p2_yz, color);
		}
	}

	void WireframeRenderer::drawCapsule(const Mat4& transform, float radius, float halfHeight, const Vec4& color) {
		// -- Cylindrical Body --
		for (int i = 0; i < WIREFRAME_SEGMENTS; i++) {
			int next = (i + 1) % WIREFRAME_SEGMENTS;

			Vec3 top1 = transform * ((g_unitCircleXZ[i] * radius) + Vec3(0.0f, halfHeight, 0.0f));
			Vec3 top2 = transform * ((g_unitCircleXZ[next] * radius) + Vec3(0.0f, halfHeight, 0.0f));
			Renderer2D::drawLine(top1, top2, color);

			Vec3 btm1 = transform * ((g_unitCircleXZ[i] * radius) - Vec3(0.0f, halfHeight, 0.0f));
			Vec3 btm2 = transform * ((g_unitCircleXZ[next] * radius) - Vec3(0.0f, halfHeight, 0.0f));
			Renderer2D::drawLine(btm1, btm2, color);

			if (i % (WIREFRAME_SEGMENTS / 4) == 0) {
				Renderer2D::drawLine(top1, btm1, color);
			}
		}

		int halfSegments = WIREFRAME_SEGMENTS / 2;

		// -- Top Hemisphere --
		for (int i = 0; i < halfSegments; i++) {
			int next = i + 1;

			Vec3 t1_xy = transform * (Vec3(g_unitCircleXY[i].x * radius, g_unitCircleXY[i].y * radius + halfHeight, 0.0f));
			Vec3 t2_xy = transform * (Vec3(g_unitCircleXY[next].x * radius, g_unitCircleXY[next].y * radius + halfHeight, 0.0f));
			Renderer2D::drawLine(t1_xy, t2_xy, color);

			Vec3 t1_zy = transform * (Vec3(0.0f, g_unitCircleXY[i].y * radius + halfHeight, g_unitCircleXY[i].x * radius));
			Vec3 t2_zy = transform * (Vec3(0.0f, g_unitCircleXY[next].y * radius + halfHeight, g_unitCircleXY[next].x * radius));
			Renderer2D::drawLine(t1_zy, t2_zy, color);
		}

		// -- Bottom Hemisphere --
		for (int i = halfSegments; i < WIREFRAME_SEGMENTS; i++) {
			int next = (i + 1) % WIREFRAME_SEGMENTS;

			Vec3 b1_xy = transform * (Vec3(g_unitCircleXY[i].x * radius, g_unitCircleXY[i].y * radius - halfHeight, 0.0f));
			Vec3 b2_xy = transform * (Vec3(g_unitCircleXY[next].x * radius, g_unitCircleXY[next].y * radius - halfHeight, 0.0f));
			Renderer2D::drawLine(b1_xy, b2_xy, color);

			Vec3 b1_zy = transform * (Vec3(0.0f, g_unitCircleXY[i].y * radius - halfHeight, g_unitCircleXY[i].x * radius));
			Vec3 b2_zy = transform * (Vec3(0.0f, g_unitCircleXY[next].y * radius - halfHeight, g_unitCircleXY[next].x * radius));
			Renderer2D::drawLine(b1_zy, b2_zy, color);
		}
	}

	void WireframeRenderer::drawMesh(const Mat4& transform, const Ref<Mesh>& mesh, const Vec4& color) {
		if (!mesh) return;

		const auto& vertices = mesh->getVertices();
		const auto& indices = mesh->getIndices();

		if (vertices.empty() || indices.empty()) return;

		for (size_t i = 0; i < indices.size(); i += 3) {
			uint32_t i0 = indices[i];
			uint32_t i1 = indices[i + 1];
			uint32_t i2 = indices[i + 2];

			Vec3 p0 = { vertices[i0].position.x, vertices[i0].position.y, vertices[i0].position.z };
			Vec3 p1 = { vertices[i1].position.x, vertices[i1].position.y, vertices[i1].position.z };
			Vec3 p2 = { vertices[i2].position.x, vertices[i2].position.y, vertices[i2].position.z };

			p0 = transform * p0;
			p1 = transform * p1;
			p2 = transform * p2;

			Renderer2D::drawLine(p0, p1, color);
			Renderer2D::drawLine(p1, p2, color);
			Renderer2D::drawLine(p2, p0, color);
		}
	}

}
