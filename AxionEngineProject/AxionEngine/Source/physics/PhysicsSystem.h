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

	class PhysicsSystem {
	public:

		static void initialize();
		static void shutdown();

		static void onSceneStart(Scene* scene);
		static void onSceneStop(Scene* scene);

		static void step(Scene* scene, Timestep ts);

	};

}
