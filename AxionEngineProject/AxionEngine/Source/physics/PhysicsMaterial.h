#pragma once

namespace Axion {

	class PhysicsMaterial {
	public:

		PhysicsMaterial() = default;
		PhysicsMaterial(float sf, float df, float rest) : staticFriction(sf), dynamicFriction(df), restitution(rest) {}
		~PhysicsMaterial() {
			if (runtimeMaterial && releaseCallback) {
				releaseCallback(runtimeMaterial);
				runtimeMaterial = nullptr;
			}
		}

		float staticFriction = 0.5f;
		float dynamicFriction = 0.5f;
		float restitution = 0.05f;

		void* runtimeMaterial = nullptr;

		void (*releaseCallback)(void*) = nullptr;
	};

}
