#include "axpch.h"
#include "PhysicsSystem.h"

#include "AxionEngine/Source/scene/Scene.h"
#include "AxionEngine/Source/scene/Components.h"

#include "AxionEngine/Vendor/physx/include/PxPhysicsAPI.h"

namespace Axion {

	using namespace physx;

	static PxDefaultAllocator s_allocator;
	static PxDefaultErrorCallback s_errorCallbak;
	static PxFoundation* s_foundation = nullptr;
	static PxPhysics* s_physics = nullptr;
	static PxDefaultCpuDispatcher* s_dispatcher = nullptr;
	static PxScene* s_physXScene = nullptr;

	void PhysicsSystem::initialize() {
		s_foundation = PxCreateFoundation(PX_PHYSICS_VERSION, s_allocator, s_errorCallbak);

		// Maybe add PhysX Visual Debugger setup

		s_physics = PxCreatePhysics(PX_PHYSICS_VERSION, *s_foundation, PxTolerancesScale(), true, nullptr);
		PxInitExtensions(*s_physics, nullptr);
		s_dispatcher = PxDefaultCpuDispatcherCreate(2); // num worker threads

		AX_CORE_LOG_INFO("Physics system initialized");
	}

	void PhysicsSystem::shutdown() {
		if (s_dispatcher) s_dispatcher->release();
		PxCloseExtensions();
		if (s_physics) s_physics->release();
		if (s_foundation) s_foundation->release();
		AX_CORE_LOG_INFO("Physics system shutdown");
	}

	void PhysicsSystem::onSceneStart(Scene* scene) {
		PxSceneDesc sceneDesc(s_physics->getTolerancesScale());
		Vec3 gravity = scene->getGravity();
		sceneDesc.gravity = PxVec3(gravity.x, gravity.y, gravity.z);
		sceneDesc.cpuDispatcher = s_dispatcher;
		sceneDesc.filterShader = PxDefaultSimulationFilterShader;
		sceneDesc.flags |= PxSceneFlag::eENABLE_CCD;

		s_physXScene = s_physics->createScene(sceneDesc);
	}

	void PhysicsSystem::onSceneStop() {
		if (s_physXScene) {
			s_physXScene->release();
			s_physXScene = nullptr;
		}
	}

	void PhysicsSystem::simulate(Scene* scene, Timestep ts) {
		if (!s_physXScene) return;

		auto& registry = scene->getRegistry();
		auto group = registry.group<RigidBodyComponent>(entt::get<TransformComponent>);

		for (auto entity : group) {
			auto& [rb, transform] = group.get<RigidBodyComponent, TransformComponent>(entity);
			if (rb.type == RigidBodyComponent::BodyType::Dynamic && rb.isKinematic && rb.runtimeActor) {
				PxRigidDynamic* actor = static_cast<PxRigidDynamic*>(rb.runtimeActor);

				float magSq = transform.rotation.x * transform.rotation.x +
					transform.rotation.y * transform.rotation.y +
					transform.rotation.z * transform.rotation.z +
					transform.rotation.w * transform.rotation.w;

				PxQuat pxQuat;
				if (magSq < 0.0001f) {
					pxQuat = PxQuat(physx::PxIdentity);
				}
				else {
					pxQuat = PxQuat(transform.rotation.x, transform.rotation.y, transform.rotation.z, transform.rotation.w).getNormalized();
				}

				PxTransform targetPose(
					PxVec3(transform.position.x, transform.position.y, transform.position.z),
					pxQuat.getNormalized()
				);

				actor->setKinematicTarget(targetPose);
			}
		}

		s_physXScene->simulate(ts.getSeconds());
		s_physXScene->fetchResults(true);

		for (auto entity : group) {
			auto& [rb, transform] = group.get<RigidBodyComponent, TransformComponent>(entity);

			if (rb.type == RigidBodyComponent::BodyType::Dynamic && rb.runtimeActor) {
				PxRigidDynamic* actor = static_cast<PxRigidDynamic*>(rb.runtimeActor);
				PxTransform physxTransform = actor->getGlobalPose();

				transform.position = { physxTransform.p.x, physxTransform.p.y, physxTransform.p.z };

				Quat q(physxTransform.q.x, physxTransform.q.y, physxTransform.q.z, physxTransform.q.w);
				transform.rotation = q;
			}
		}

	}

	PxPhysics* PhysicsSystem::getPhysics() { return s_physics; }
	PxScene* PhysicsSystem::getScene() { return s_physXScene; }

}
