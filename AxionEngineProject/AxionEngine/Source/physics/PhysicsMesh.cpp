#include "axpch.h"
#include "PhysicsMesh.h"

#include <physx/include/PxPhysicsAPI.h>

namespace Axion {

	PhysicsMesh::PhysicsMesh(Type type, void* runtimeMesh)
		: m_type(type), m_runtimeMesh(runtimeMesh) {}

	PhysicsMesh::~PhysicsMesh() {
		release();
	}

	void PhysicsMesh::release() {
		if (m_runtimeMesh) {
			if (m_type == Type::Convex) {
				static_cast<physx::PxConvexMesh*>(m_runtimeMesh)->release();
			}
			else if (m_type == Type::Triangle) {
				static_cast<physx::PxTriangleMesh*>(m_runtimeMesh)->release();
			}
			m_runtimeMesh = nullptr;
		}
	}

}
