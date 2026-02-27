#include "axpch.h"
#include "Physics.h"

#include "AxionEngine/Source/scene/Components.h"

#include "AxionEngine/Vendor/physx/include/PxPhysicsAPI.h"

namespace Axion {

	static physx::PxForceMode::Enum getPhysXForceMode(ForceMode mode) {
		switch (mode) {
			case Axion::ForceMode::Force: return physx::PxForceMode::eFORCE;
			case Axion::ForceMode::Impulse: return physx::PxForceMode::eIMPULSE;
			case Axion::ForceMode::VelocityChange: return physx::PxForceMode::eVELOCITY_CHANGE;
			case Axion::ForceMode::Acceleration: return physx::PxForceMode::eACCELERATION;
		}
		return physx::PxForceMode::eFORCE;
	}

	void Physics::addForce(Entity entity, const Vec3& force, ForceMode mode) {
		if (!entity.hasComponent<RigidBodyComponent>()) return;

		auto& rb = entity.getComponent<RigidBodyComponent>();

		if (rb.type == RigidBodyComponent::BodyType::Dynamic && rb.runtimeActor) {
			physx::PxRigidDynamic* actor = static_cast<physx::PxRigidDynamic*>(rb.runtimeActor);

			actor->addForce(physx::PxVec3(force.x, force.y, force.z), getPhysXForceMode(mode));
		}
	}

	void Physics::addTorgue(Entity entity, const Vec3& torgue, ForceMode mode) {
		if (!entity.hasComponent<RigidBodyComponent>()) return;

		auto& rb = entity.getComponent<RigidBodyComponent>();

		if (rb.type == RigidBodyComponent::BodyType::Dynamic && rb.runtimeActor) {
			physx::PxRigidDynamic* actor = static_cast<physx::PxRigidDynamic*>(rb.runtimeActor);

			actor->addTorque(physx::PxVec3(torgue.x, torgue.y, torgue.z), getPhysXForceMode(mode));
		}
	}

	void Physics::addRadialImpulse(Entity entity, const Vec3& origin, float radius, float strength) {
		if (!entity.hasComponent<RigidBodyComponent>()) return;

		auto& transform = entity.getComponent<TransformComponent>();
		Vec3 direction = transform.position - origin;
		float distance = direction.length();

		if (distance < radius) {
			direction.normalized();
			float falloff = 1.0f - (distance / radius);

			Vec3 impulse = direction * strength * falloff;
			addForce(entity, impulse, ForceMode::Impulse);
		}
	}

	void Physics::setLinearVelocity(Entity entity, const Vec3& velocity) {
		if (!entity.hasComponent<RigidBodyComponent>()) return;

		auto& rb = entity.getComponent<RigidBodyComponent>();

		if (rb.type == RigidBodyComponent::BodyType::Dynamic && rb.runtimeActor) {
			physx::PxRigidDynamic* actor = static_cast<physx::PxRigidDynamic*>(rb.runtimeActor);
			actor->setLinearVelocity(physx::PxVec3(velocity.x, velocity.y, velocity.z));
		}
	}

	Vec3 Physics::getLinearVelocity(Entity entity) {
		if (!entity.hasComponent<RigidBodyComponent>()) return Vec3::zero();

		auto& rb = entity.getComponent<RigidBodyComponent>();

		if (rb.type == RigidBodyComponent::BodyType::Dynamic && rb.runtimeActor) {
			physx::PxRigidDynamic* actor = static_cast<physx::PxRigidDynamic*>(rb.runtimeActor);
			physx::PxVec3 vel = actor->getLinearVelocity();
			return { vel.x, vel.y, vel.z };
		}

		return Vec3::zero();
	}

	void Physics::setAngularVelocity(Entity entity, const Vec3& velocity) {
		if (!entity.hasComponent<RigidBodyComponent>()) return;

		auto& rb = entity.getComponent<RigidBodyComponent>();

		if (rb.type == RigidBodyComponent::BodyType::Dynamic && rb.runtimeActor) {
			physx::PxRigidDynamic* actor = static_cast<physx::PxRigidDynamic*>(rb.runtimeActor);
			actor->setAngularVelocity(physx::PxVec3(velocity.x, velocity.y, velocity.z));
		}
	}

	Vec3 Physics::getAngularVelocity(Entity entity) {
		if (!entity.hasComponent<RigidBodyComponent>()) return Vec3::zero();

		auto& rb = entity.getComponent<RigidBodyComponent>();

		if (rb.type == RigidBodyComponent::BodyType::Dynamic && rb.runtimeActor) {
			physx::PxRigidDynamic* actor = static_cast<physx::PxRigidDynamic*>(rb.runtimeActor);
			physx::PxVec3 vel = actor->getAngularVelocity();
			return { vel.x, vel.y, vel.z };
		}

		return Vec3::zero();
	}

	void Physics::setMass(Entity entity, float mass) {
		if (!entity.hasComponent<RigidBodyComponent>()) return;

		auto& rb = entity.getComponent<RigidBodyComponent>();

		rb.mass = mass;

		if (rb.type == RigidBodyComponent::BodyType::Dynamic && rb.runtimeActor) {
			physx::PxRigidDynamic* actor = static_cast<physx::PxRigidDynamic*>(rb.runtimeActor);
			physx::PxRigidBodyExt::updateMassAndInertia(*actor, rb.mass);
		}
	}

	float Physics::getMass(Entity entity) {
		if (!entity.hasComponent<RigidBodyComponent>()) return 0.0f;

		auto& rb = entity.getComponent<RigidBodyComponent>();

		return rb.mass;
	}

}
