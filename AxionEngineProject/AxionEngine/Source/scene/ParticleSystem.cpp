#include "axpch.h"
#include "ParticleSystem.h"

#include "AxionEngine/Source/scene/Components.h"

namespace Axion {

	void ParticleSystem::emitParticle(ParticleSystemComponent& psc, const Vec3& spawnPosition) {
		ParticleProps& particle = psc.particlePool[psc.poolIndex];
		particle.active = true;
		particle.position = spawnPosition;
	
		particle.velocity.x = Math::randomFloat(-psc.velocityVariation.x, psc.velocityVariation.x);
		particle.velocity.y = Math::randomFloat(-psc.velocityVariation.y, psc.velocityVariation.y);
		particle.velocity.z = Math::randomFloat(-psc.velocityVariation.z, psc.velocityVariation.z);

		particle.colorBegin = psc.colorBegin;
		particle.colorEnd = psc.colorEnd;
		particle.sizeBegin = psc.sizeBegin;
		particle.sizeEnd = psc.sizeEnd;
		particle.lifeTime = psc.lifeTime;
		particle.lifeRemaining = psc.lifeTime;

		psc.poolIndex = --psc.poolIndex % psc.particlePool.size();
	}

}
