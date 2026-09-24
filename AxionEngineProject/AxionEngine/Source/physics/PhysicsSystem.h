#pragma once

#include "AxionEngine/Source/core/Timestep.h"
#include "AxionEngine/Source/core/Math.h"
#include "AxionEngine/Source/scene/Entity.h"

namespace Axion {

	class Scene;

	struct Collision {
		Entity other;
		Vec3 contactPoint;
		Vec3 contactNormal;
		Vec3 impulse;
	};

	struct RaycastHit {
		Entity entity;
		Vec3 position;
		Vec3 normal;
		float distance;
	};

	class PhysicsSystem {
	public:

		static void initialize();
		static void shutdown();

		static void onSceneStart(Scene* scene);
		static void onSceneStop(Scene* scene);

		static void step(Scene* scene, Timestep ts);
		static void destroyBody(Entity entity);

		static bool raycast(Scene* scene, const Vec3& origin, const Vec3& direction, float maxDistance, RaycastHit* outHit);
		static bool sweepBox(Scene* scene, const Vec3& origin, const Vec3& halfExtents, const Vec3& orientation, const Vec3& direction, float maxDistance, RaycastHit* outHit);
		static bool sweepSphere(Scene* scene, const Vec3& origin, float radius, const Vec3& direction, float maxDistance, RaycastHit* outHit);
		static bool sweepCapsule(Scene* scene, const Vec3& origin, float radius, float halfHeight, const Vec3& orientation, const Vec3& direction, float maxDistance, RaycastHit* outHit);
		static size_t overlapBox(Scene* scene, const Vec3& center, const Vec3& halfExtents, const Vec3& orientation, uint64_t* outIdsHi, uint64_t* outIdsLo, size_t maxUUIDs);
		static size_t overlapSphere(Scene* scene, const Vec3& center, float radius, uint64_t* outIdsHi, uint64_t* outIdsLo, size_t maxUUIDs);
		static size_t overlapCapsule(Scene* scene, const Vec3& center, float radius, float halfHeight, const Vec3& orientation, uint64_t* outIdsHi, uint64_t* outIdsLo, size_t maxUUIDs);

	private:

		static void createPhysicsActor(Entity entity, Scene* scene, RigidBodyComponent& rb, TransformComponent& transform);

	};

}
