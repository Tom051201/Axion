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

	private:

		static void createPhysicsActor(Entity entity, Scene* scene, RigidBodyComponent& rb, TransformComponent& transform);

	};

}
