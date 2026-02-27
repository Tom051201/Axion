#include "axpch.h"
#include "PhysicsSystem.h"

#include "AxionEngine/Source/scene/Scene.h"
#include "AxionEngine/Source/scene/Components.h"
#include "AxionEngine/Source/physics/PhysicsMaterial.h"
#include "AxionEngine/Source/core/AssetManager.h"

#include "AxionEngine/Vendor/physx/include/PxPhysicsAPI.h"

namespace Axion {

	using namespace physx;

	static PxDefaultAllocator s_allocator;
	static PxDefaultErrorCallback s_errorCallbak;
	static PxFoundation* s_foundation = nullptr;
	static PxPhysics* s_physics = nullptr;
	static PxDefaultCpuDispatcher* s_dispatcher = nullptr;
	static PxScene* s_physXScene = nullptr;
	static PxMaterial* s_defaultMaterial = nullptr;

	PxMaterial* getOrCreatePhysXMaterial(AssetHandle<PhysicsMaterial> handle) {
		Ref<PhysicsMaterial> phyMatAsset = AssetManager::get<PhysicsMaterial>(handle);

		// -- Fallback --
		if (!phyMatAsset) {
			return s_defaultMaterial;
		}

		// -- Return cached --
		if (phyMatAsset->runtimeMaterial) {
			return static_cast<PxMaterial*>(phyMatAsset->runtimeMaterial);
		}

		// -- Create and cache it --
		PxMaterial* newMaterial = s_physics->createMaterial(
			phyMatAsset->staticFriction,
			phyMatAsset->dynamicFriction,
			phyMatAsset->restitution
		);
		phyMatAsset->runtimeMaterial = newMaterial;

		// -- Cleanup callback --
		phyMatAsset->releaseCallback = [](void* mat) {
			static_cast<PxMaterial*>(mat)->release();
		};

		return newMaterial;
	}

	void PhysicsSystem::initialize() {
		s_foundation = PxCreateFoundation(PX_PHYSICS_VERSION, s_allocator, s_errorCallbak);

		// Maybe add PhysX Visual Debugger setup

		s_physics = PxCreatePhysics(PX_PHYSICS_VERSION, *s_foundation, PxTolerancesScale(), true, nullptr);
		PxInitExtensions(*s_physics, nullptr);
		s_dispatcher = PxDefaultCpuDispatcherCreate(2); // num worker threads

		s_defaultMaterial = s_physics->createMaterial(0.5f, 0.5f, 0.05f);

		AX_CORE_LOG_INFO("Physics system initialized");
	}

	void PhysicsSystem::shutdown() {
		if (s_defaultMaterial) s_defaultMaterial->release();
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

		auto group = scene->getRegistry().group<RigidBodyComponent>(entt::get<TransformComponent>);
		for (auto entity : group) {
			auto& [rb, transform] = group.get<RigidBodyComponent, TransformComponent>(entity);

			PxQuat pxQuat(transform.rotation.x, transform.rotation.y, transform.rotation.z, transform.rotation.w);
			if (pxQuat.magnitudeSquared() < 0.0001) {
				pxQuat = PxQuat(PxIdentity);
			}

			PxTransform physxTransform(
				PxVec3(transform.position.x, transform.position.y, transform.position.z),
				pxQuat.getNormalized()
			);

			PxRigidActor* actor = nullptr;
			if (rb.type == RigidBodyComponent::BodyType::Static) {
				actor = s_physics->createRigidStatic(physxTransform);
			}
			else {
				PxRigidDynamic* dynamicActor = s_physics->createRigidDynamic(physxTransform);
				dynamicActor->setRigidBodyFlag(PxRigidBodyFlag::eKINEMATIC, rb.isKinematic);
				dynamicActor->setRigidBodyFlag(PxRigidBodyFlag::eENABLE_CCD, rb.enableCCD);
				dynamicActor->setLinearDamping(rb.linearDamping);
				dynamicActor->setAngularDamping(rb.angularDamping);

				dynamicActor->setRigidDynamicLockFlag(PxRigidDynamicLockFlag::eLOCK_ANGULAR_X, rb.fixedRotationX);
				dynamicActor->setRigidDynamicLockFlag(PxRigidDynamicLockFlag::eLOCK_ANGULAR_Y, rb.fixedRotationY);
				dynamicActor->setRigidDynamicLockFlag(PxRigidDynamicLockFlag::eLOCK_ANGULAR_Z, rb.fixedRotationZ);

				actor = dynamicActor;
			}

			if (scene->getRegistry().all_of<BoxColliderComponent>(entity)) {
				auto& bc = scene->getRegistry().get<BoxColliderComponent>(entity);

				PxMaterial* material = getOrCreatePhysXMaterial(bc.material);

				PxVec3 offset = {
					bc.offset.x * transform.scale.x,
					bc.offset.y * transform.scale.y,
					bc.offset.z * transform.scale.z
				};

				PxVec3 halfExtents = {
					std::abs(bc.halfExtents.x * transform.scale.x),
					std::abs(bc.halfExtents.y * transform.scale.y),
					std::abs(bc.halfExtents.z * transform.scale.z)
				};

				if (halfExtents.x < 0.001f) halfExtents.x = 0.001f;
				if (halfExtents.y < 0.001f) halfExtents.y = 0.001f;
				if (halfExtents.z < 0.001f) halfExtents.z = 0.001f;

				PxShape* shape = PxRigidActorExt::createExclusiveShape(*actor, PxBoxGeometry(halfExtents), *material);
				shape->setLocalPose(PxTransform(offset));

				bc.runtimeShape = shape;
			}

			if (scene->getRegistry().all_of<SphereColliderComponent>(entity)) {
				auto& sc = scene->getRegistry().get<SphereColliderComponent>(entity);

				PxMaterial* material = getOrCreatePhysXMaterial(sc.material);

				float maxScale = std::max(std::abs(transform.scale.x), std::max(std::abs(transform.scale.y), std::abs(transform.scale.z)));
				float geometryRadius = sc.radius * maxScale;

				PxVec3 offset = {
					sc.offset.x * transform.scale.x,
					sc.offset.y * transform.scale.y,
					sc.offset.z * transform.scale.z
				};

				if (geometryRadius < 0.001f) geometryRadius = 0.001f;

				PxShape* shape = PxRigidActorExt::createExclusiveShape(*actor, PxSphereGeometry(geometryRadius), *material);
				shape->setLocalPose(PxTransform(offset));

				sc.runtimeShape = shape;
			}

			if (scene->getRegistry().all_of<CapsuleColliderComponent>(entity)) {
				auto& cc = scene->getRegistry().get<CapsuleColliderComponent>(entity);

				PxMaterial* material = getOrCreatePhysXMaterial(cc.material);

				float scaleXZ = std::max(std::abs(transform.scale.x), std::abs(transform.scale.z));
				float scaledRadius = cc.radius * scaleXZ;
				float scaledHalfHeight = cc.halfHeight * std::abs(transform.scale.y);

				if (scaledRadius < 0.001f) scaledRadius = 0.001f;
				if (scaledHalfHeight < 0.001f) scaledHalfHeight = 0.001f;

				PxShape* shape = PxRigidActorExt::createExclusiveShape(*actor, PxCapsuleGeometry(scaledRadius, scaledHalfHeight), *material);

				PxQuat relativeRotation(PxHalfPi, PxVec3(0.0f, 0.0f, 1.0f));

				PxVec3 offset = {
					cc.offset.x * transform.scale.x,
					cc.offset.y * transform.scale.y,
					cc.offset.z * transform.scale.z
				};

				shape->setLocalPose(PxTransform(offset, relativeRotation));

				cc.runtimeShape = shape;
			}

			if (rb.type == RigidBodyComponent::BodyType::Dynamic) {
				PxRigidBodyExt::updateMassAndInertia(*(PxRigidDynamic*)actor, rb.mass);
			}

			s_physXScene->addActor(*actor);
			rb.runtimeActor = actor;
		}
	}

	void PhysicsSystem::onSceneStop(Scene* scene) {
		auto view = scene->getRegistry().view<RigidBodyComponent>();
		for (auto entity : view) {
			auto& rb = view.get<RigidBodyComponent>(entity);
			if (rb.runtimeActor) {
				static_cast<PxRigidActor*>(rb.runtimeActor)->release();
				rb.runtimeActor = nullptr;
			}
		}

		if (s_physXScene) {
			s_physXScene->release();
			s_physXScene = nullptr;
		}
	}

	void PhysicsSystem::step(Scene* scene, Timestep ts) {
		if (!s_physXScene) return;

		auto& registry = scene->getRegistry();

		auto bodyGroup = registry.group<RigidBodyComponent>(entt::get<TransformComponent>);
		auto gravityGroup = registry.group<GravitySourceComponent>(entt::get<TransformComponent>);

		for (auto entity : bodyGroup) {
			auto& [rb, transform] = bodyGroup.get<RigidBodyComponent, TransformComponent>(entity);

			if (rb.type != RigidBodyComponent::BodyType::Dynamic || rb.isKinematic || !rb.runtimeActor) {
				continue;
			}

			PxRigidDynamic* actor = static_cast<PxRigidDynamic*>(rb.runtimeActor);
			PxVec3 targetPos(transform.position.x, transform.position.y, transform.position.z);

			PxVec3 totalForce(0.0f);
			bool isAffectedByField = false;

			for (auto sourceEntity : gravityGroup) {
				if (entity == sourceEntity) continue;

				auto& [source, sourceTransform] = gravityGroup.get<GravitySourceComponent, TransformComponent>(sourceEntity);
				PxVec3 sourcePos(sourceTransform.position.x, sourceTransform.position.y, sourceTransform.position.z);

				if (source.type == GravitySourceComponent::Type::Directional) {
					PxVec3 defaultDir(0.0f, -1.0f, 0.0f);
					PxQuat q(sourceTransform.rotation.x, sourceTransform.rotation.y, sourceTransform.rotation.z, sourceTransform.rotation.w);

					totalForce += q.rotate(defaultDir) * source.strength * rb.mass;
					isAffectedByField = true;
				}
				else if (source.type == GravitySourceComponent::Type::Point) {
					PxVec3 direction = sourcePos - targetPos;
					float distance = direction.magnitude();

					if (distance < source.radius && distance > 0.01f) {
						direction.normalize();

						totalForce += direction * source.strength * rb.mass;
						isAffectedByField = true;
					}
				}
			}

			if (isAffectedByField) {
				actor->addForce(totalForce, PxForceMode::eFORCE);
				actor->setActorFlag(PxActorFlag::eDISABLE_GRAVITY, true);
			}
			else {
				actor->setActorFlag(PxActorFlag::eDISABLE_GRAVITY, !rb.useGlobalGravity);
			}
		}

		for (auto entity : bodyGroup) {
			auto& [rb, transform] = bodyGroup.get<RigidBodyComponent, TransformComponent>(entity);

			if (rb.isKinematic && rb.runtimeActor) {
				PxRigidDynamic* actor = static_cast<PxRigidDynamic*>(rb.runtimeActor);

				PxQuat q(transform.rotation.x, transform.rotation.y, transform.rotation.z, transform.rotation.w);
				PxTransform target(
					PxVec3(transform.position.x, transform.position.y, transform.position.z),
					q.getNormalized()
				);
				actor->setKinematicTarget(target);
			}
		}

		s_physXScene->simulate(ts.getSeconds());
		s_physXScene->fetchResults(true);

		for (auto entity : bodyGroup) {
			auto& [rb, transform] = bodyGroup.get<RigidBodyComponent, TransformComponent>(entity);

			if (rb.type == RigidBodyComponent::BodyType::Dynamic && !rb.isKinematic && rb.runtimeActor) {
				PxRigidDynamic* actor = static_cast<PxRigidDynamic*>(rb.runtimeActor);
				PxTransform pt = actor->getGlobalPose();

				transform.position = { pt.p.x, pt.p.y, pt.p.z };
				transform.rotation = { pt.q.x, pt.q.y, pt.q.z, pt.q.w };
			}
		}
	}

}
