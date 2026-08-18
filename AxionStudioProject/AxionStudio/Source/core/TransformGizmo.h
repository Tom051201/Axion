#pragma once

#include <optional>

#include "AxionEngine/Source/core/Math.h"
#include "AxionEngine/Source/graphics/Camera.h"
#include "AxionEngine/Source/scene/Entity.h"

namespace Axion {

	enum class GizmoMode { Translate, Rotate, Scale };
	enum class GizmoAxis { None, X, Y, Z, XY, XZ, YZ, Screen };
	enum class GizmoSpace { Local, Global };

	class TransformGizmo {
	public:

		TransformGizmo() = default;
		~TransformGizmo() = default;

		void setMode(GizmoMode mode) { m_mode = mode; }
		GizmoMode getMode() const { return m_mode; }

		void setSpace(GizmoSpace space) { m_space = space; }
		GizmoSpace getSpace() const { return m_space; }

		std::optional<Vec3> onUpdate(const Mat4& entityWorldTransform, const Camera& camera, const Vec2& mousePos, const Vec2& viewportSize, bool isMouseDown, bool snap = false, float snapValue = 1.0f);
		void onRender(const Mat4& entityWorldTransform, const Camera& camera);

	private:

		GizmoMode m_mode = GizmoMode::Translate;
		GizmoSpace m_space = GizmoSpace::Local;
		GizmoAxis m_hoveredAxis = GizmoAxis::None;
		GizmoAxis m_activeAxis = GizmoAxis::None;

		bool m_isDragging = false;
		Vec3 m_initialIntersectionPoint;
		Vec3 m_originalEntityPosition;

		Vec3 m_initialVector;
		float m_currentAngle = 0.0f;
		Vec3 m_dragPlaneNormal;

		float m_sX = 1.0f;
		float m_sY = 1.0f;
		float m_sZ = 1.0f;

		void calculateHoveredAxis(const Vec3& rayOrigin, const Vec3& rayDir, const Vec3& entityPos, const Quat& rotation, float gizmoSize);
		Vec3 intersectRayWithPlane(const Vec3& rayOrigin, const Vec3& rayDir, const Vec3& planeOrigin, const Vec3& planeNormal);

	};

}
