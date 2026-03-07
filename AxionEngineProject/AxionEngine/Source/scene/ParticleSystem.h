#pragma once

#include "AxionEngine/Source/core/Math.h"

namespace Axion {

	struct ParticleProps {
		Vec3 position = Vec3::zero();
		Vec3 velocity = Vec3::zero();
		Vec4 colorBegin = Vec4::one();
		Vec4 colorEnd = Vec4::one();
		float sizeBegin = 1.0f;
		float sizeEnd = 0.0f;
		float lifeTime = 1.0f;
		float lifeRemaining = 0.0f;
		bool active = false;
	};



	struct ParticleSystemComponent;

	class ParticleSystem {
	public:

		static void emitParticle(ParticleSystemComponent& psc, const Vec3& spawnPosition);

	};

}
