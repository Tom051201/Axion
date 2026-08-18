#include "studiopch.h"
#include "TransformGizmo.h"

#include "AxionEngine/Source/graphics/Renderer2D.h"

#include "AxionStudio/Source/core/WireframeRenderer.h"

namespace Axion {

	std::optional<Vec3> TransformGizmo::onUpdate(const Mat4& entityWorldTransform, const Camera& camera, const Vec2& mousePos, const Vec2& viewportSize, bool isMouseDown, bool snap, float snapValue) {
		float ndcX = (mousePos.x / viewportSize.x) * 2.0f - 1.0f;
		float ndcY = 1.0f - (mousePos.y / viewportSize.y) * 2.0f;

		Mat4 invVP = camera.getViewProjectionMatrix().inverse();
		Vec4 rayStart = invVP * Vec4(ndcX, ndcY, 0.0f, 1.0f);
		Vec4 rayEnd = invVP * Vec4(ndcX, ndcY, 1.0f, 1.0f);

		Vec3 rayOrigin = Vec3(rayStart.x, rayStart.y, rayStart.z) / rayStart.w;
		Vec3 rayTarget = Vec3(rayEnd.x, rayEnd.y, rayEnd.z) / rayEnd.w;
		Vec3 rayDir = (rayTarget - rayOrigin).normalized();

		Vec3 worldPos = entityWorldTransform.getTranslation();
		Quat rot = (m_space == GizmoSpace::Local) ? entityWorldTransform.getRotation() : Quat::identity();

		Vec3 camPos = camera.getViewMatrix().inverse().getTranslation();
		float gizmoSize = (camPos - worldPos).length() * 0.15f;

		if (!isMouseDown) {
			Vec3 viewDirLocal = rot.inversed().rotate((camPos - worldPos).normalized());
			m_sX = viewDirLocal.x >= 0.0f ? 1.0f : -1.0f;
			m_sY = viewDirLocal.y >= 0.0f ? 1.0f : -1.0f;
			m_sZ = viewDirLocal.z >= 0.0f ? 1.0f : -1.0f;

			m_isDragging = false;
			m_activeAxis = GizmoAxis::None;
			calculateHoveredAxis(rayOrigin, rayDir, worldPos, rot, gizmoSize);
			return std::nullopt;
		}

		if (m_hoveredAxis != GizmoAxis::None || m_activeAxis != GizmoAxis::None) {
			GizmoAxis axisToUse = m_isDragging ? m_activeAxis : m_hoveredAxis;

			if (m_mode == GizmoMode::Translate || m_mode == GizmoMode::Scale) {
				Vec3 localAxisDir = Vec3::zero();
				Vec3 localPlaneNormal = Vec3::zero();
				bool isPlanar = false;

				if (axisToUse == GizmoAxis::X) {
					localAxisDir = Vec3(m_sX, 0, 0);
					localPlaneNormal = Vec3(0, m_sY, 0);
				}
				if (axisToUse == GizmoAxis::Y) {
					localAxisDir = Vec3(0, m_sY, 0);
					localPlaneNormal = Vec3(m_sX, 0, 0);
				}
				if (axisToUse == GizmoAxis::Z) {
					localAxisDir = Vec3(0, 0, m_sZ);
					localPlaneNormal = Vec3(0, m_sY, 0);
				}
				if (axisToUse == GizmoAxis::XY) {
					localPlaneNormal = Vec3(0, 0, m_sZ);
					isPlanar = true;
				}
				if (axisToUse == GizmoAxis::XZ) {
					localPlaneNormal = Vec3(0, m_sY, 0);
					isPlanar = true;
				}
				if (axisToUse == GizmoAxis::YZ) {
					localPlaneNormal = Vec3(m_sX, 0, 0);
					isPlanar = true;
				}

				Vec3 worldAxisDir = rot.rotate(localAxisDir);
				Vec3 worldPlaneNormal = rot.rotate(localPlaneNormal);

				if (!isPlanar) {
					Vec3 viewDir = (worldPos - camPos).normalized();
					Vec3 planeRight = viewDir.cross(worldAxisDir);
					if (planeRight.length() < 0.001f) planeRight = (axisToUse == GizmoAxis::Y) ? rot.rotate(Vec3(1, 0, 0)) : rot.rotate(Vec3(0, 1, 0));
					worldPlaneNormal = worldAxisDir.cross(planeRight).normalized();
				}

				if (!m_isDragging) {
					m_activeAxis = m_hoveredAxis;
					m_isDragging = true;
					m_originalEntityPosition = worldPos;
					m_dragPlaneNormal = worldPlaneNormal;
					m_initialIntersectionPoint = intersectRayWithPlane(rayOrigin, rayDir, worldPos, m_dragPlaneNormal);
					return std::nullopt;
				}
				else {
					Vec3 currentHitPoint = intersectRayWithPlane(rayOrigin, rayDir, m_originalEntityPosition, m_dragPlaneNormal);
					Vec3 delta = currentHitPoint - m_initialIntersectionPoint;

					Vec3 totalMoveWorld = Vec3::zero();
					Vec3 totalMoveLocal = Vec3::zero();

					if (isPlanar) {
						totalMoveWorld = delta;
						totalMoveLocal = rot.inversed().rotate(delta);
						if (axisToUse == GizmoAxis::XY) totalMoveLocal.z = 0.0f;
						if (axisToUse == GizmoAxis::XZ) totalMoveLocal.y = 0.0f;
						if (axisToUse == GizmoAxis::YZ) totalMoveLocal.x = 0.0f;
					}
					else {
						float moveAmount = delta.dot(worldAxisDir);
						totalMoveLocal = localAxisDir * moveAmount;
					}

					if (snap) {
						if (snapValue <= 0.0f) snapValue = 0.1f;
						totalMoveLocal.x = std::round(totalMoveLocal.x / snapValue) * snapValue;
						totalMoveLocal.y = std::round(totalMoveLocal.y / snapValue) * snapValue;
						totalMoveLocal.z = std::round(totalMoveLocal.z / snapValue) * snapValue;
					}

					totalMoveWorld = rot.rotate(totalMoveLocal);

					if (m_mode == GizmoMode::Translate) {
						Vec3 newWorldPos = m_originalEntityPosition + totalMoveWorld;
						return newWorldPos - worldPos;
					}
					else {
						m_initialIntersectionPoint = currentHitPoint;
						return totalMoveLocal * 0.5f;
					}
				}
			}
			else if (m_mode == GizmoMode::Rotate) {
				Vec3 localAxisDir = Vec3::zero();
				if (axisToUse == GizmoAxis::X) localAxisDir = Vec3(m_sX, 0, 0);
				if (axisToUse == GizmoAxis::Y) localAxisDir = Vec3(0, m_sY, 0);
				if (axisToUse == GizmoAxis::Z) localAxisDir = Vec3(0, 0, m_sZ);

				Vec3 worldAxisDir = rot.rotate(localAxisDir);
				Vec3 worldPlaneNormal = worldAxisDir;

				if (!m_isDragging) {
					m_activeAxis = m_hoveredAxis;
					m_isDragging = true;
					m_currentAngle = 0.0f;
					m_dragPlaneNormal = worldPlaneNormal;
					Vec3 hitPoint = intersectRayWithPlane(rayOrigin, rayDir, worldPos, m_dragPlaneNormal);
					m_initialVector = (hitPoint - worldPos).normalized();
					return std::nullopt;
				}
				else {
					Vec3 hitPoint = intersectRayWithPlane(rayOrigin, rayDir, worldPos, m_dragPlaneNormal);
					Vec3 currentVector = (hitPoint - worldPos).normalized();

					float dot = Math::clamp(m_initialVector.dot(currentVector), -1.0f, 1.0f);
					float angle = std::acos(dot);

					Vec3 cross = m_initialVector.cross(currentVector);
					if (cross.dot(worldPlaneNormal) < 0.0f) angle = -angle;

					float totalAngleDegrees = m_currentAngle + Math::toDegrees(angle);

					if (snap) {
						float sVal = snapValue > 0.0f ? snapValue : 15.0f;
						totalAngleDegrees = std::round(totalAngleDegrees / sVal) * sVal;
					}

					float deltaAngle = totalAngleDegrees - m_currentAngle;

					if (!snap || std::abs(deltaAngle) > 0.01f) {
						m_currentAngle += deltaAngle;
						m_initialVector = currentVector;
					}

					return worldAxisDir * deltaAngle;
				}
			}
		}

		return std::nullopt;
	}

	void TransformGizmo::onRender(const Mat4& entityWorldTransform, const Camera& camera) {
		Vec3 worldPos = entityWorldTransform.getTranslation();
		Quat rot = (m_space == GizmoSpace::Local) ? entityWorldTransform.getRotation() : Quat::identity();

		Mat4 invView = camera.getViewMatrix().inverse();
		Vec3 camPos = invView.getTranslation();

		float distance = (camPos - worldPos).length();
		float gizmoSize = distance * 0.15f;
		float planeOffset = gizmoSize * 0.15f;
		float planeSize = gizmoSize * 0.4f;

		Vec4 colorX = (m_hoveredAxis == GizmoAxis::X || m_activeAxis == GizmoAxis::X) ? Vec4(1, 1, 0, 1) : Vec4(1, 0, 0, 1);
		Vec4 colorY = (m_hoveredAxis == GizmoAxis::Y || m_activeAxis == GizmoAxis::Y) ? Vec4(1, 1, 0, 1) : Vec4(0, 1, 0, 1);
		Vec4 colorZ = (m_hoveredAxis == GizmoAxis::Z || m_activeAxis == GizmoAxis::Z) ? Vec4(1, 1, 0, 1) : Vec4(0, 0, 1, 1);
		Vec4 colorXY = (m_hoveredAxis == GizmoAxis::XY || m_activeAxis == GizmoAxis::XY) ? Vec4(1, 1, 0, 0.5f) : Vec4(0, 0, 1, 0.5f);
		Vec4 colorXZ = (m_hoveredAxis == GizmoAxis::XZ || m_activeAxis == GizmoAxis::XZ) ? Vec4(1, 1, 0, 0.5f) : Vec4(0, 1, 0, 0.5f);
		Vec4 colorYZ = (m_hoveredAxis == GizmoAxis::YZ || m_activeAxis == GizmoAxis::YZ) ? Vec4(1, 1, 0, 0.5f) : Vec4(1, 0, 0, 0.5f);

		Renderer2D::beginScene(camera);

		float thickness = gizmoSize * 0.05f;
		float headSize = gizmoSize * 0.15f;
		Mat4 gizmoBase = Mat4::TRS(worldPos, rot, Vec3::one());

		auto drawSolidBox = [&](const Mat4& transform, const Vec4& color) {
			Vec4 cX = { color.x * 0.6f, color.y * 0.6f, color.z * 0.6f, color.w };
			Vec4 cY = { color.x * 0.8f, color.y * 0.8f, color.z * 0.8f, color.w };
			Vec4 cZ = { color.x, color.y, color.z, color.w };

			Renderer2D::drawQuad(Mat4::TRS(Vec3(0, 0, 0.5f), Quat::identity(), Vec3(1, 1, 1)) * transform, cZ);
			Renderer2D::drawQuad(Mat4::TRS(Vec3(0, 0, -0.5f), Quat::fromEulerAngles(Vec3(0, 180.0f, 0)), Vec3(1, 1, 1)) * transform, cZ);
			Renderer2D::drawQuad(Mat4::TRS(Vec3(0.5f, 0, 0), Quat::fromEulerAngles(Vec3(0, 90.0f, 0)), Vec3(1, 1, 1)) * transform, cX);
			Renderer2D::drawQuad(Mat4::TRS(Vec3(-0.5f, 0, 0), Quat::fromEulerAngles(Vec3(0, -90.0f, 0)), Vec3(1, 1, 1)) * transform, cX);
			Renderer2D::drawQuad(Mat4::TRS(Vec3(0, 0.5f, 0), Quat::fromEulerAngles(Vec3(-90.0f, 0, 0)), Vec3(1, 1, 1)) * transform, cY);
			Renderer2D::drawQuad(Mat4::TRS(Vec3(0, -0.5f, 0), Quat::fromEulerAngles(Vec3(90.0f, 0, 0)), Vec3(1, 1, 1)) * transform, cY);
			};

		if (m_mode == GizmoMode::Translate || m_mode == GizmoMode::Scale) {

			auto drawLineAndHead = [&](const Vec3& axis, const Vec4& color) {
				Mat4 lineM = Mat4::TRS(axis * gizmoSize * 0.5f, Quat::identity(), Vec3(
					std::abs(axis.x) > 0.1f ? gizmoSize : thickness,
					std::abs(axis.y) > 0.1f ? gizmoSize : thickness,
					std::abs(axis.z) > 0.1f ? gizmoSize : thickness
				)) * gizmoBase;
				drawSolidBox(lineM, color);

				Mat4 headM = Mat4::TRS(axis * gizmoSize, Quat::identity(), Vec3(headSize, headSize, headSize)) * gizmoBase;
				drawSolidBox(headM, color);
			};

			drawLineAndHead(Vec3(m_sX, 0, 0), colorX);
			drawLineAndHead(Vec3(0, m_sY, 0), colorY);
			drawLineAndHead(Vec3(0, 0, m_sZ), colorZ);

			Vec3 xyPos = Vec3((planeOffset + planeSize * 0.5f) * m_sX, (planeOffset + planeSize * 0.5f) * m_sY, 0);
			Renderer2D::drawQuad(Mat4::TRS(xyPos, Quat::identity(), Vec3(planeSize, planeSize, 1.0f)) * gizmoBase, colorXY);
			Renderer2D::drawQuad(Mat4::TRS(xyPos, Quat::fromEulerAngles(Vec3(0, 180.0f, 0)), Vec3(planeSize, planeSize, 1.0f)) * gizmoBase, colorXY);

			Vec3 xzPos = Vec3((planeOffset + planeSize * 0.5f) * m_sX, 0, (planeOffset + planeSize * 0.5f) * m_sZ);
			Renderer2D::drawQuad(Mat4::TRS(xzPos, Quat::fromEulerAngles(Vec3(90.0f, 0.0f, 0.0f)), Vec3(planeSize, planeSize, 1.0f)) * gizmoBase, colorXZ);
			Renderer2D::drawQuad(Mat4::TRS(xzPos, Quat::fromEulerAngles(Vec3(-90.0f, 0.0f, 0.0f)), Vec3(planeSize, planeSize, 1.0f)) * gizmoBase, colorXZ);

			Vec3 yzPos = Vec3(0, (planeOffset + planeSize * 0.5f) * m_sY, (planeOffset + planeSize * 0.5f) * m_sZ);
			Renderer2D::drawQuad(Mat4::TRS(yzPos, Quat::fromEulerAngles(Vec3(0.0f, 90.0f, 0.0f)), Vec3(planeSize, planeSize, 1.0f)) * gizmoBase, colorYZ);
			Renderer2D::drawQuad(Mat4::TRS(yzPos, Quat::fromEulerAngles(Vec3(0.0f, -90.0f, 0.0f)), Vec3(planeSize, planeSize, 1.0f)) * gizmoBase, colorYZ);
		}
		else if (m_mode == GizmoMode::Rotate) {

			auto drawSolidRing = [&](const Vec3& normal, const Vec4& color) {
				Vec3 up = (std::abs(normal.y) > 0.99f) ? Vec3(1, 0, 0) : Vec3(0, 1, 0);
				Vec3 right = normal.cross(up).normalized();
				up = right.cross(normal).normalized();

				const int segments = 48;
				float segLen = (DirectX::XM_2PI * gizmoSize) / segments * 1.05f;

				Vec3 prevPoint = right * gizmoSize;

				for (int i = 1; i <= segments; i++) {
					float angle = (float)i / segments * 2.0f * DirectX::XM_PI;
					Vec3 point = right * (std::cos(angle) * gizmoSize) + up * (std::sin(angle) * gizmoSize);

					Vec3 midPoint = (prevPoint + point) * 0.5f;
					Vec3 tangent = (point - prevPoint).normalized();
					Vec3 bitangent = normal.cross(tangent).normalized();

					DirectX::XMMATRIX xmRot = DirectX::XMMatrixIdentity();
					xmRot.r[0] = DirectX::XMVectorSet(tangent.x, tangent.y, tangent.z, 0.0f);
					xmRot.r[1] = DirectX::XMVectorSet(normal.x, normal.y, normal.z, 0.0f);
					xmRot.r[2] = DirectX::XMVectorSet(bitangent.x, bitangent.y, bitangent.z, 0.0f);

					Mat4 rotM = Mat4::fromXM(xmRot);
					Mat4 sM = Mat4::scale(Vec3(segLen, thickness * 0.2f, thickness * 1.5f));
					Mat4 tM = Mat4::translation(midPoint);

					drawSolidBox(sM * rotM * tM * gizmoBase, color);
					prevPoint = point;
				}
			};

			drawSolidRing(Vec3(1, 0, 0), colorX);
			drawSolidRing(Vec3(0, 1, 0), colorY);
			drawSolidRing(Vec3(0, 0, 1), colorZ);
		}

		if (m_isDragging && m_mode == GizmoMode::Translate) {
			Vec4 white = Vec4(1.0f, 1.0f, 1.0f, 1.0f);
			Renderer2D::drawLine(m_originalEntityPosition, worldPos, white);

			float originalDistance = (camPos - m_originalEntityPosition).length();
			float dotSize = (originalDistance * 0.15f) * 0.08f;

			Mat4 dotTransform = Mat4::TRS(m_originalEntityPosition, Quat::identity(), Vec3(dotSize, dotSize, dotSize));
			drawSolidBox(dotTransform, white);
		}

		Renderer2D::endScene();
	}

	Vec3 TransformGizmo::intersectRayWithPlane(const Vec3& rayOrigin, const Vec3& rayDir, const Vec3& planeOrigin, const Vec3& planeNormal) {
		float denom = rayDir.dot(planeNormal);
		if (std::abs(denom) > 1e-6f) {
			float t = (planeOrigin - rayOrigin).dot(planeNormal) / denom;
			return rayOrigin + (rayDir * t);
		}
		return planeOrigin;
	}

	void TransformGizmo::calculateHoveredAxis(const Vec3& rayOrigin, const Vec3& rayDir, const Vec3& entityPos, const Quat& rotation, float gizmoSize) {
		m_hoveredAxis = GizmoAxis::None;
		float thicknessThreshold = gizmoSize * 0.1f;
		float closestDist = FLT_MAX;

		Mat4 invGizmoBase = Mat4::TRS(entityPos, rotation, Vec3::one()).inverse();
		Vec3 localRayOrigin = (invGizmoBase * Vec4(rayOrigin.x, rayOrigin.y, rayOrigin.z, 1.0f)).xyz();
		Vec3 localRayDir = (invGizmoBase * Vec4(rayDir.x, rayDir.y, rayDir.z, 0.0f)).xyz().normalized();
		Vec3 center = Vec3::zero();

		if (m_mode == GizmoMode::Translate || m_mode == GizmoMode::Scale) {
			float planeOffset = gizmoSize * 0.15f;
			float planeSize = gizmoSize * 0.4f;

			auto checkPlane = [&](const Vec3& normal, GizmoAxis axisType) {
				Vec3 hitPoint = intersectRayWithPlane(localRayOrigin, localRayDir, center, normal);
				Vec3 mappedHit = Vec3(hitPoint.x * m_sX, hitPoint.y * m_sY, hitPoint.z * m_sZ);

				bool inside = false;
				if (axisType == GizmoAxis::XY) inside = (mappedHit.x > planeOffset && mappedHit.x < planeOffset + planeSize && mappedHit.y > planeOffset && mappedHit.y < planeOffset + planeSize);
				if (axisType == GizmoAxis::XZ) inside = (mappedHit.x > planeOffset && mappedHit.x < planeOffset + planeSize && mappedHit.z > planeOffset && mappedHit.z < planeOffset + planeSize);
				if (axisType == GizmoAxis::YZ) inside = (mappedHit.y > planeOffset && mappedHit.y < planeOffset + planeSize && mappedHit.z > planeOffset && mappedHit.z < planeOffset + planeSize);

				if (inside) {
					float dist = (hitPoint - localRayOrigin).length();
					if (dist < closestDist) {
						closestDist = dist;
						m_hoveredAxis = axisType;
					}
				}
			};

			checkPlane(Vec3(0, 0, 1), GizmoAxis::XY);
			checkPlane(Vec3(0, 1, 0), GizmoAxis::XZ);
			checkPlane(Vec3(1, 0, 0), GizmoAxis::YZ);

			auto checkAxis = [&](const Vec3& axisDir, GizmoAxis axisType) {
				float a = 1.0f, b = localRayDir.dot(axisDir), c = 1.0f;
				float d = localRayDir.dot(localRayOrigin), e = axisDir.dot(localRayOrigin);
				float denom = a * c - b * b;
				if (denom < 0.0001f) return;

				float sc = (b * e - c * d) / denom;
				float tc = (a * e - b * d) / denom;

				if (tc < 0.0f) tc = 0.0f;
				if (tc > gizmoSize) tc = gizmoSize;

				Vec3 pointOnRay = localRayOrigin + (localRayDir * sc);
				Vec3 pointOnAxis = center + (axisDir * tc);
				float dist = (pointOnRay - pointOnAxis).length();

				if (dist < thicknessThreshold && dist < closestDist) {
					closestDist = dist;
					m_hoveredAxis = axisType;
				}
			};

			checkAxis(Vec3(m_sX, 0.0f, 0.0f), GizmoAxis::X);
			checkAxis(Vec3(0.0f, m_sY, 0.0f), GizmoAxis::Y);
			checkAxis(Vec3(0.0f, 0.0f, m_sZ), GizmoAxis::Z);
		}
		else if (m_mode == GizmoMode::Rotate) {

			auto checkRing = [&](const Vec3& axisNormal, GizmoAxis axisType) {
				Vec3 hitPoint = intersectRayWithPlane(localRayOrigin, localRayDir, center, axisNormal);
				float radius = hitPoint.length();

				float dist = std::abs(radius - gizmoSize);
				if (dist < thicknessThreshold && dist < closestDist) {
					closestDist = dist;
					m_hoveredAxis = axisType;
				}
			};

			checkRing(Vec3(1.0f, 0.0f, 0.0f), GizmoAxis::X);
			checkRing(Vec3(0.0f, 1.0f, 0.0f), GizmoAxis::Y);
			checkRing(Vec3(0.0f, 0.0f, 1.0f), GizmoAxis::Z);
		}
	}

}
