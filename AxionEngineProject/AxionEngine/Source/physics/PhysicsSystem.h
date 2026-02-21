#pragma once

#include "AxionEngine/Source/core/Timestep.h"

namespace Axion {

	class Scene;

	class PhysicsSystem {
	public:

		static void initialize();
		static void shutdown();

		static void onSceneStart(Scene* scene);
		static void onSceneStop();

		static void simulate(Scene* scene, Timestep ts);

	};

}
