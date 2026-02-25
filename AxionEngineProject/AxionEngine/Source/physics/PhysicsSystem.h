#pragma once

#include "AxionEngine/Source/core/Timestep.h"

namespace Axion {

	class Scene;

	class PhysicsSystem {
	public:

		static void initialize();
		static void shutdown();

		static void onSceneStart(Scene* scene);
		static void onSceneStop(Scene* scene);

		static void step(Scene* scene, Timestep ts);

	};

}
