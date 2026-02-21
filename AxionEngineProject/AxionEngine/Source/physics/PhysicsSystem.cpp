#include "axpch.h"
#include "PhysicsSystem.h"

#include "AxionEngine/Source/scene/Scene.h"
#include "AxionEngine/Source/scene/Components.h"

namespace Axion {

	void PhysicsSystem::initialize() {

	}

	void PhysicsSystem::shutdown() {

	}

	void PhysicsSystem::onSceneStart(Scene* scene) {

	}

	void PhysicsSystem::onSceneStop() {

	}

	void PhysicsSystem::simulate(Scene* scene, Timestep ts) {
		auto& registry = scene->getRegistry();

		// simulate

		// sync
		auto group = registry.group<RigidBodyComponent>(entt::get<TransformComponent>);
		for (auto entity : group) {
			auto& [rb, transform] = group.get<RigidBodyComponent, TransformComponent>(entity);

			if (rb.type == RigidBodyComponent::BodyType::Dynamic && rb.runtimeActor) {
				//physx::PxRigidDynamic* actor = static_cast<physx::PxRigidDynamic*>(rb.runtimeActor);
				//physx::PxTransform physxTransform = actor->getGlobalPose();

				//transform.position = { physxTransform.p.x, physxTransform.p.y, physxTransform.p.z };
				//transform.rotation = Math::QuatToEuler(physxTransform.q);
			}
		}


	}

}
