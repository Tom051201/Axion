#include "WireframeRenderer.h"
#include "AxionEngine/Source/render/Renderer2D.h"

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

		// -- Hemispheres --
		int halfSegments = WIREFRAME_SEGMENTS / 2;
		for (int i = 0; i < halfSegments; i++) {
			int next = (i + 1) % WIREFRAME_SEGMENTS;

			Vec3 top_xy1 = transform * ((g_unitCircleXY[i] * radius) + Vec3(0.0f, halfHeight, 0.0f));
			Vec3 top_xy2 = transform * ((g_unitCircleXY[next] * radius) + Vec3(0.0f, halfHeight, 0.0f));
			Renderer2D::drawLine(top_xy1, top_xy2, color);

			Vec3 top_yz1 = transform * ((g_unitCircleYZ[i] * radius) + Vec3(0.0f, halfHeight, 0.0f));
			Vec3 top_yz2 = transform * ((g_unitCircleYZ[next] * radius) + Vec3(0.0f, halfHeight, 0.0f));
			Renderer2D::drawLine(top_yz1, top_yz2, color);

			int btmIdx = i + halfSegments;
			int btmNext = (btmIdx + 1) % WIREFRAME_SEGMENTS;

			Vec3 btm_xy1 = transform * ((g_unitCircleXY[btmIdx] * radius) - Vec3(0.0f, halfHeight, 0.0f));
			Vec3 btm_xy2 = transform * ((g_unitCircleXY[btmNext] * radius) - Vec3(0.0f, halfHeight, 0.0f));
			Renderer2D::drawLine(btm_xy1, btm_xy2, color);

			Vec3 btm_yz1 = transform * ((g_unitCircleYZ[btmIdx] * radius) - Vec3(0.0f, halfHeight, 0.0f));
			Vec3 btm_yz2 = transform * ((g_unitCircleYZ[btmNext] * radius) - Vec3(0.0f, halfHeight, 0.0f));
			Renderer2D::drawLine(btm_yz1, btm_yz2, color);
		}
	}

}
