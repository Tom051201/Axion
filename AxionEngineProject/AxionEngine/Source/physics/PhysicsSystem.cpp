#include "axpch.h"
#include "PhysicsSystem.h"

#include <physx/include/PxPhysicsAPI.h>
#include <physx/include/cooking/PxCooking.h>
#include <physx/include/characterkinematic/PxControllerManager.h>
#include <physx/include/characterkinematic/PxController.h>
#include <physx/include/extensions/PxFixedJoint.h>
#include <physx/include/extensions/PxDistanceJoint.h>
#include <physx/include/extensions/PxRevoluteJoint.h>

#include "AxionEngine/Source/core/AssetManager.h"
#include "AxionEngine/Source/scene/Scene.h"
#include "AxionEngine/Source/scene/Components.h"
#include "AxionEngine/Source/scene/ScriptableEntity.h"
#include "AxionEngine/Source/physics/PhysicsMaterial.h"
#include "AxionEngine/Source/scripting/ScriptEngine.h"

namespace Axion {

	using namespace physx;

	static PxDefaultAllocator s_allocator;
	static PxDefaultErrorCallback s_errorCallbak;
	static PxFoundation* s_foundation = nullptr;
	static PxPhysics* s_physics = nullptr;
	static PxDefaultCpuDispatcher* s_dispatcher = nullptr;
	static PxScene* s_physXScene = nullptr;
	static PxMaterial* s_defaultMaterial = nullptr;
	static PxControllerManager* s_cctManager = nullptr;

	static PxFilterFlags AxionSimulatorFilterShader(
		PxFilterObjectAttributes attributes0, PxFilterData filterData0,
		PxFilterObjectAttributes attributes1, PxFilterData filterData1,
		PxPairFlags& pairFlags, const void* constantBlock, PxU32 constantBlockSize)
	{
		// -- Let triggers bypass the collision solver --
		if (PxFilterObjectIsTrigger(attributes0) || PxFilterObjectIsTrigger(attributes1)) {
			pairFlags = PxPairFlag::eTRIGGER_DEFAULT;
			return PxFilterFlag::eDEFAULT;
		}

		// -- Bitwise Layer & Mask Check --
		// filterData.word0 = The object's Layer
		// filterData.word1 = The object's Collision Mask
		bool collide = (filterData0.word0 & filterData1.word1) && (filterData1.word0 & filterData0.word1);

		if (collide) {
			pairFlags = PxPairFlag::eCONTACT_DEFAULT;
			pairFlags |= PxPairFlag::eNOTIFY_TOUCH_FOUND;
			pairFlags |= PxPairFlag::eNOTIFY_TOUCH_LOST;
			pairFlags |= PxPairFlag::eNOTIFY_TOUCH_CCD;
			pairFlags |= PxPairFlag::eNOTIFY_CONTACT_POINTS;
			return PxFilterFlag::eDEFAULT;
		}

		// -- Ignore collision entirely if masks don't match --
		return PxFilterFlag::eKILL;
	}

	class CharacterHitCallback : public physx::PxUserControllerHitReport {
	public:

		void onShapeHit(const physx::PxControllerShapeHit& hit) override {
			Scene* scene = ScriptEngine::getSceneContext();
			if (!scene) return;

			entt::entity playerHandle = (entt::entity)(uintptr_t)hit.controller->getUserData();
			Entity playerEntity = { playerHandle, scene };

			if (!hit.shape || !hit.actor || !hit.actor->userData) return;
			entt::entity otherHandle = (entt::entity)(uintptr_t)hit.actor->userData;
			Entity otherEntity = { otherHandle, scene };

			if (!playerEntity.isValid() || !otherEntity.isValid()) return;

			if (hit.actor->is<physx::PxRigidDynamic>()) {
				physx::PxRigidDynamic* dynamicActor = static_cast<physx::PxRigidDynamic*>(hit.actor);

				if (!(dynamicActor->getRigidBodyFlags() & physx::PxRigidBodyFlag::eKINEMATIC)) {
					float pushPower = 50.0f;
					if (playerEntity.hasComponent<CharacterControllerComponent>()) {
						pushPower = playerEntity.getComponent<CharacterControllerComponent>().pushPower;
					}

					physx::PxVec3 pushDirection = -hit.worldNormal;
					pushDirection.y = 0.0f;
					pushDirection.normalize();

					physx::PxRigidBodyExt::addForceAtPos(
						*dynamicActor,
						pushDirection * pushPower,
						physx::PxVec3(static_cast<float>(hit.worldPos.x), static_cast<float>(hit.worldPos.y), static_cast<float>(hit.worldPos.z)),
						physx::PxForceMode::eIMPULSE
					);
				}
			}

			if (playerEntity.hasComponent<ScriptComponent>()) {
				void* gcHandle = playerEntity.getComponent<ScriptComponent>().gcHandle;
				if (gcHandle) {
					Collision collision;
					collision.other = otherEntity;
					collision.contactPoint = { (float)hit.worldPos.x, (float)hit.worldPos.y, (float)hit.worldPos.z };
					collision.contactNormal = { (float)hit.worldNormal.x, (float)hit.worldNormal.y, (float)hit.worldNormal.z };
					collision.impulse = { 0.0f, (float)hit.length, 0.0f };

					ScriptEngine::onCollisionEnter(gcHandle, collision);
				}
			}
		}

		void onControllerHit(const physx::PxControllersHit& hit) override {
			// Called when two CCTs bump into each other. You can implement this similarly if you have NPC CCTs.
		}

		void onObstacleHit(const physx::PxControllerObstacleHit& hit) override {}

	};

	static CharacterHitCallback s_cctHitCallback;

	class PhysicsContactListener : public PxSimulationEventCallback {
	public:

		Scene* currentScene = nullptr;

		void onConstraintBreak(PxConstraintInfo* constraints, PxU32 count) override {}
		void onWake(PxActor** actors, PxU32 count) override {}
		void onSleep(PxActor** actors, PxU32 count) override {}
		void onAdvance(const PxRigidBody* const* bodyBuffer, const PxTransform* poseBuffer, const PxU32 count) override {}

		void onContact(const PxContactPairHeader& pairHeader, const PxContactPair* pairs, PxU32 nbPairs) override {
			if (!currentScene) return;

			entt::entity handleA = (entt::entity)(uintptr_t)pairHeader.actors[0]->userData;
			entt::entity handleB = (entt::entity)(uintptr_t)pairHeader.actors[1]->userData;

			Entity entityA = { handleA, currentScene };
			Entity entityB = { handleB, currentScene };

			if (!entityA.isValid() || !entityB.isValid()) return;

			for (PxU32 i = 0; i < nbPairs; i++) {
				const PxContactPair& cp = pairs[i];

				Vec3 contactPoint = Vec3::zero();
				Vec3 contactNormal = Vec3::zero();
				Vec3 impulse = Vec3::zero();

				if (cp.contactCount > 0) {
					// -- Only use the primary contact point --
					PxContactPairPoint primaryContact;
					cp.extractContacts(&primaryContact, 1);

					contactPoint = { primaryContact.position.x, primaryContact.position.y, primaryContact.position.z };
					contactNormal = { primaryContact.normal.x, primaryContact.normal.y, primaryContact.normal.z };
					impulse = { primaryContact.impulse.x, primaryContact.impulse.y, primaryContact.impulse.z };
				}

				if (cp.events & PxPairFlag::eNOTIFY_TOUCH_FOUND) {
					currentScene->queueCollision(handleA, handleB, contactPoint, contactNormal, impulse, true);
					currentScene->queueCollision(handleB, handleA, contactPoint, { -contactNormal.x, -contactNormal.y, -contactNormal.z }, impulse, true);
				}
				else if (cp.events & PxPairFlag::eNOTIFY_TOUCH_LOST) {
					currentScene->queueCollision(handleA, handleB, contactPoint, contactNormal, impulse, false);
					currentScene->queueCollision(handleB, handleA, contactPoint, { -contactNormal.x, -contactNormal.y, -contactNormal.z }, impulse, false);
				}

			}
		}

		void onTrigger(PxTriggerPair* pairs, PxU32 count) override {
			if (!currentScene) return;

			for (PxU32 i = 0; i < count; i++) {
				const PxTriggerPair& tp = pairs[i];

				// -- Ignore deleted shapes --
				if (tp.flags & (PxTriggerPairFlag::eREMOVED_SHAPE_TRIGGER | PxTriggerPairFlag::eREMOVED_SHAPE_OTHER))
					continue;

				entt::entity triggerHandle = (entt::entity)(uintptr_t)tp.triggerActor->userData;
				entt::entity otherHandle = (entt::entity)(uintptr_t)tp.otherActor->userData;

				Entity triggerEntity = { triggerHandle, currentScene };
				if (!triggerEntity.isValid()) continue;

				if (tp.status == PxPairFlag::eNOTIFY_TOUCH_FOUND) {
					currentScene->queueTrigger(triggerHandle, otherHandle, true);
				}
				else if (tp.status == PxPairFlag::eNOTIFY_TOUCH_LOST) {
					currentScene->queueTrigger(triggerHandle, otherHandle, false);
				}

			}

		}

	};

	static PhysicsContactListener s_contactListener;

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
		s_contactListener.currentScene = scene;

		PxSceneDesc sceneDesc(s_physics->getTolerancesScale());
		Vec3 gravity = scene->getGravity();
		sceneDesc.gravity = PxVec3(gravity.x, gravity.y, gravity.z);
		sceneDesc.cpuDispatcher = s_dispatcher;

		sceneDesc.filterShader = AxionSimulatorFilterShader;
		sceneDesc.simulationEventCallback = &s_contactListener;

		sceneDesc.flags |= PxSceneFlag::eENABLE_CCD;

		s_physXScene = s_physics->createScene(sceneDesc);
		s_cctManager = PxCreateControllerManager(*s_physXScene);

		auto rbView = scene->getRegistry().view<RigidBodyComponent, TransformComponent>();
		for (auto [entityHandle, rb, transform] : rbView.each()) {

			Entity entity = { entityHandle, scene };
			createPhysicsActor(entity, scene, rb, transform);
		}

	}

	void PhysicsSystem::onSceneStop(Scene* scene) {
		// -- Release Physics Joints --
		auto jointView = scene->getRegistry().view<PhysicsJointComponent>();
		for (auto entity : jointView) {
			auto& jc = jointView.get<PhysicsJointComponent>(entity);
			if (jc.runtimeJoint) {
				static_cast<physx::PxJoint*>(jc.runtimeJoint)->release();
				jc.runtimeJoint = nullptr;
			}
		}

		// -- Release physx actor --
		auto view = scene->getRegistry().view<RigidBodyComponent>();
		for (auto entity : view) {
			auto& rb = view.get<RigidBodyComponent>(entity);
			if (rb.runtimeActor) {
				static_cast<PxRigidActor*>(rb.runtimeActor)->release();
				rb.runtimeActor = nullptr;
			}
		}

		// -- Release controller manager --
		if (s_cctManager) {
			s_cctManager->release();
			s_cctManager = nullptr;
		}

		// -- Release physx scene --
		if (s_physXScene) {
			s_physXScene->release();
			s_physXScene = nullptr;
		}
	}

	struct ActiveGravitySource {
		entt::entity entityHandle;
		GravitySourceComponent::Type type;
		PxVec3 position;
		PxVec3 direction;
		float strength;
		float radius;
	};

	static void setupShapeFilterData(PxShape* shape, uint32_t layer, uint32_t mask) {
		PxFilterData filterData;
		filterData.word0 = layer; // Object's ID
		filterData.word1 = mask;  // What it can hit

		shape->setSimulationFilterData(filterData);
		shape->setQueryFilterData(filterData);
	}

	void PhysicsSystem::step(Scene* scene, Timestep ts) {
		if (!s_physXScene) return;

		auto& registry = scene->getRegistry();
		auto rbView = registry.view<RigidBodyComponent, TransformComponent>();
		auto gravityView = registry.view<GravitySourceComponent, TransformComponent>();



		// ----- Pre-Calculate Gravity Sources -----
		std::vector<ActiveGravitySource> activeGravitySources;
		activeGravitySources.reserve(16);
		for (auto [sourceEntity, source, sourceTransform] : gravityView.each()) {
			ActiveGravitySource data;
			data.entityHandle = sourceEntity;
			data.type = source.type;
			data.strength = source.strength;
			data.radius = source.radius;
			data.position = PxVec3(sourceTransform.position.x, sourceTransform.position.y, sourceTransform.position.z);

			if (source.type == GravitySourceComponent::Type::Directional) {
				PxVec3 defaultDir(0.0f, -1.0f, 0.0f);
				PxQuat q(sourceTransform.rotation.x, sourceTransform.rotation.y, sourceTransform.rotation.z, sourceTransform.rotation.w);
				data.direction = q.rotate(defaultDir);
			}

			activeGravitySources.push_back(data);
		}



		// ----- Pre-Simulation Pass -----
		for (auto [entityHandle, rb, transform] : rbView.each()) {

			// -- Late initialization --
			if (rb.runtimeActor == nullptr) {
				Entity entity = { entityHandle, scene };
				createPhysicsActor(entity, scene, rb, transform);
			}

			// -- Safty check --
			if (!rb.runtimeActor) continue;

			// -- Process Dynamic Bodies --
			if (rb.type == RigidBodyComponent::BodyType::Dynamic) {
				PxRigidDynamic* actor = static_cast<PxRigidDynamic*>(rb.runtimeActor);

				// -- Update kinematic target --
				if (rb.isKinematic) {
					PxQuat q(transform.rotation.x, transform.rotation.y, transform.rotation.z, transform.rotation.w);
					PxTransform target(
						PxVec3(transform.position.x, transform.position.y, transform.position.z),
						q.getNormalized()
					);
					actor->setKinematicTarget(target);
				}
				else {
					// -- Apply gravity fields / forces to standard dynamic bodies --
					PxVec3 targetPos(transform.position.x, transform.position.y, transform.position.z);
					PxVec3 totalForce(0.0f);
					bool isAffectedByField = false;

					for (const auto& source : activeGravitySources) {
						if (entityHandle == source.entityHandle) continue;

						if (source.type == GravitySourceComponent::Type::Directional) {
							totalForce += source.direction * source.strength * rb.mass;
							isAffectedByField = true;
						}
						else if (source.type == GravitySourceComponent::Type::Point) {
							PxVec3 direction = source.position - targetPos;
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
			}
		}


		// ----- Joints Pre-Simulation Pass -----
		auto jointView = registry.view<PhysicsJointComponent, RigidBodyComponent>();
		for (auto [entityHandle, jc, rb] : jointView.each()) {
			if (jc.runtimeJoint == nullptr && rb.runtimeActor != nullptr) {

				physx::PxRigidActor* actor0 = static_cast<physx::PxRigidActor*>(rb.runtimeActor);
				physx::PxRigidActor* actor1 = nullptr;

				if (jc.connectedEntity.isValid()) {
					Entity connectedEnt = scene->getEntityByUUID(jc.connectedEntity);
					if (connectedEnt.isValid() && connectedEnt.hasComponent<RigidBodyComponent>()) {
						auto& connectedRb = connectedEnt.getComponent<RigidBodyComponent>();
						if (connectedRb.runtimeActor) {
							actor1 = static_cast<physx::PxRigidActor*>(connectedRb.runtimeActor);
						}
						else {
							continue;
						}
					}
				}

				physx::PxTransform localFrame0(physx::PxVec3(jc.localAnchor1.x, jc.localAnchor1.y, jc.localAnchor1.z));
				physx::PxTransform localFrame1(physx::PxVec3(jc.localAnchor2.x, jc.localAnchor2.y, jc.localAnchor2.z));

				if (jc.localAnchor2 == Vec3(0.0f, 0.0f, 0.0f) && actor1 != nullptr) {
					physx::PxTransform worldAnchor = actor0->getGlobalPose() * localFrame0;
					localFrame1.p = actor1->getGlobalPose().getInverse().transform(worldAnchor.p);
				}
				physx::PxJoint* joint = nullptr;

				switch (jc.type) {
					case JointType::Fixed: {
						joint = physx::PxFixedJointCreate(*s_physics, actor0, localFrame0, actor1, localFrame1);
						break;
					}
					case JointType::Distance: {
						physx::PxDistanceJoint* distJoint = physx::PxDistanceJointCreate(*s_physics, actor0, localFrame0, actor1, localFrame1);

						distJoint->setMinDistance(jc.minDistance);
						distJoint->setMaxDistance(jc.maxDistance);
						distJoint->setDistanceJointFlag(physx::PxDistanceJointFlag::eMIN_DISTANCE_ENABLED, jc.minDistance > 0.0f);
						distJoint->setDistanceJointFlag(physx::PxDistanceJointFlag::eMAX_DISTANCE_ENABLED, true);

						if (jc.springStiffness > 0.0f) {
							distJoint->setStiffness(jc.springStiffness);
							distJoint->setDamping(jc.springDamping);
							distJoint->setDistanceJointFlag(physx::PxDistanceJointFlag::eSPRING_ENABLED, true);
						}

						joint = distJoint;
						break;
					}
					case JointType::Hinge: {
						physx::PxQuat upHinge(physx::PxHalfPi, physx::PxVec3(0.0f, 0.0f, 1.0f));
						localFrame0.q = upHinge;
						localFrame1.q = upHinge;

						physx::PxRevoluteJoint* hingeJoint = physx::PxRevoluteJointCreate(*s_physics, actor0, localFrame0, actor1, localFrame1);
						joint = hingeJoint;
						break;
					}
				}

				// -- Apply universal joint settings --
				if (joint) {
					joint->setConstraintFlag(physx::PxConstraintFlag::eCOLLISION_ENABLED, jc.enableCollision);

					if (jc.isBreakable) {
						joint->setBreakForce(jc.breakForce, jc.breakForce);
					}

					jc.runtimeJoint = joint;
				}
			}
		}


		// ----- CCT Pre-Simulation Pass -----
		auto cctView = registry.view<CharacterControllerComponent, TransformComponent>();
		for (auto [entityHandle, cct, transform] : cctView.each()) {
			if (cct.runtimeController == nullptr) {
				PxMaterial* material = getOrCreatePhysXMaterial(cct.material);

				PxCapsuleControllerDesc desc;
				desc.height = cct.height;
				desc.radius = cct.radius;
				desc.stepOffset = cct.stepOffset;
				desc.slopeLimit = std::cos(Math::toRadians(cct.slopeLimitDegrees));
				desc.material = material;
				desc.position = PxExtendedVec3(transform.position.x, transform.position.y, transform.position.z);
				desc.upDirection = PxVec3(0, 1, 0);
				desc.userData = (void*)(uintptr_t)entityHandle;
				desc.reportCallback = &s_cctHitCallback;

				PxController* controller = s_cctManager->createController(desc);

				PxRigidDynamic* actor = controller->getActor();
				if (actor && actor->getNbShapes() > 0) {
					PxShape* shape;
					actor->getShapes(&shape, 1);
					setupShapeFilterData(shape, cct.layer, cct.collisionMask);
				}

				cct.runtimeController = controller;
			}
		}




		// ----- Simulate physx -----
		s_physXScene->simulate(ts.getSeconds());
		s_physXScene->fetchResults(true);



		// ----- Post-Simulation Pass -----
		for (auto [entity, rb, transform] : rbView.each()) {
			if (rb.type == RigidBodyComponent::BodyType::Dynamic && !rb.isKinematic && rb.runtimeActor) {
				PxRigidDynamic* actor = static_cast<PxRigidDynamic*>(rb.runtimeActor);
				PxTransform pt = actor->getGlobalPose();

				transform.position = { pt.p.x, pt.p.y, pt.p.z };
				transform.rotation = { pt.q.x, pt.q.y, pt.q.z, pt.q.w };
			}
		}

		// ----- CCT Post-Simulation Pass -----
		for (auto [entityHandle, cct, transform] : cctView.each()) {
			if (cct.runtimeController) {
				PxController* controller = static_cast<PxController*>(cct.runtimeController);
				const PxExtendedVec3& pos = controller->getPosition();
				transform.position = { (float)pos.x, (float)pos.y, (float)pos.z };
			}
		}

	}

	void PhysicsSystem::destroyBody(Entity entity) {
		if (!s_physXScene) return;

		if (entity.hasComponent<RigidBodyComponent>()) {
			auto& rb = entity.getComponent<RigidBodyComponent>();
			if (rb.runtimeActor) {
				PxRigidActor* actor = static_cast<PxRigidActor*>(rb.runtimeActor);
				s_physXScene->removeActor(*actor);
				actor->release();
				rb.runtimeActor = nullptr;
			}
		}

	}

	bool PhysicsSystem::raycast(Scene* scene, const Vec3& origin, const Vec3& direction, float maxDistance, RaycastHit* outHit) {
		if (!s_physXScene) return false;

		if (direction.length() == 0.0f) {
			AX_CORE_LOG_WARN("PhysicsSystem::raycast called with a zero-length direction vector!");
			return false;
		}

		PxVec3 pxOrigin(origin.x, origin.y, origin.z);
		PxVec3 pxDir(direction.x, direction.y, direction.z);
		pxDir.normalize();

		PxRaycastBuffer hit;
		if (s_physXScene->raycast(pxOrigin, pxDir, maxDistance, hit)) {
			if (outHit) {
				outHit->position = { hit.block.position.x, hit.block.position.y, hit.block.position.z };
				outHit->normal = { hit.block.normal.x, hit.block.normal.y, hit.block.normal.z };
				outHit->distance = hit.block.distance;

				if (hit.block.actor) {
					entt::entity handle = (entt::entity)(uintptr_t)hit.block.actor->userData;
					outHit->entity = { handle, scene };
				}
				else {
					outHit->entity = {};
				}
			}
			return true;
		}
		return false;
	}

	static physx::PxQuat EulerToPxQuat(const Vec3& euler) {
		physx::PxQuat qX(euler.x, physx::PxVec3(1.0f, 0.0f, 0.0f));
		physx::PxQuat qY(euler.y, physx::PxVec3(0.0f, 1.0f, 0.0f));
		physx::PxQuat qZ(euler.z, physx::PxVec3(0.0f, 0.0f, 1.0f));
		return qZ * qY * qX;
	}

	bool PhysicsSystem::sweepBox(Scene* scene, const Vec3& origin, const Vec3& halfExtents, const Vec3& orientation, const Vec3& direction, float maxDistance, RaycastHit* outHit) {
		if (!s_physXScene || direction.length() == 0.0f) return false;

		physx::PxVec3 pxDir(direction.x, direction.y, direction.z); pxDir.normalize();
		physx::PxTransform pose(physx::PxVec3(origin.x, origin.y, origin.z), EulerToPxQuat(orientation));
		physx::PxBoxGeometry geom(halfExtents.x, halfExtents.y, halfExtents.z);
		physx::PxSweepBuffer hit;

		if (s_physXScene->sweep(geom, pose, pxDir, maxDistance, hit)) {
			if (outHit) {
				outHit->position = { (float)hit.block.position.x, (float)hit.block.position.y, (float)hit.block.position.z };
				outHit->normal = { hit.block.normal.x, hit.block.normal.y, hit.block.normal.z };
				outHit->distance = hit.block.distance;
				outHit->entity = (hit.block.actor && hit.block.actor->userData) ? Entity{ (entt::entity)(uintptr_t)hit.block.actor->userData, scene } : Entity{};
			}
			return true;
		}
		return false;
	}

	bool PhysicsSystem::sweepSphere(Scene* scene, const Vec3& origin, float radius, const Vec3& direction, float maxDistance, RaycastHit* outHit) {
		if (!s_physXScene) return false;

		if (direction.length() == 0.0f) return false;

		physx::PxVec3 pxOrigin(origin.x, origin.y, origin.z);
		physx::PxVec3 pxDir(direction.x, direction.y, direction.z);
		pxDir.normalize();

		physx::PxTransform pose(pxOrigin);
		physx::PxSphereGeometry geom(radius);
		physx::PxSweepBuffer hit;

		// -- Sweep shape through the scene --
		if (s_physXScene->sweep(geom, pose, pxDir, maxDistance, hit)) {
			if (outHit) {
				outHit->position = { (float)hit.block.position.x, (float)hit.block.position.y, (float)hit.block.position.z };
				outHit->normal = { hit.block.normal.x, hit.block.normal.y, hit.block.normal.z };
				outHit->distance = hit.block.distance;

				if (hit.block.actor && hit.block.actor->userData) {
					entt::entity handle = (entt::entity)(uintptr_t)hit.block.actor->userData;
					outHit->entity = { handle, scene };
				}
				else {
					outHit->entity = {};
				}
			}
			return true;
		}
		return false;
	}

	bool PhysicsSystem::sweepCapsule(Scene* scene, const Vec3& origin, float radius, float halfHeight, const Vec3& orientation, const Vec3& direction, float maxDistance, RaycastHit* outHit) {
		if (!s_physXScene || direction.length() == 0.0f) return false;

		physx::PxVec3 pxDir(direction.x, direction.y, direction.z); pxDir.normalize();
		physx::PxTransform pose(physx::PxVec3(origin.x, origin.y, origin.z), EulerToPxQuat(orientation));
		physx::PxCapsuleGeometry geom(radius, halfHeight);
		physx::PxSweepBuffer hit;

		if (s_physXScene->sweep(geom, pose, pxDir, maxDistance, hit)) {
			if (outHit) {
				outHit->position = { (float)hit.block.position.x, (float)hit.block.position.y, (float)hit.block.position.z };
				outHit->normal = { hit.block.normal.x, hit.block.normal.y, hit.block.normal.z };
				outHit->distance = hit.block.distance;
				outHit->entity = (hit.block.actor && hit.block.actor->userData) ? Entity{ (entt::entity)(uintptr_t)hit.block.actor->userData, scene } : Entity{};
			}
			return true;
		}
		return false;
	}

	size_t PhysicsSystem::overlapBox(Scene* scene, const Vec3& center, const Vec3& halfExtents, const Vec3& orientation, uint64_t* outIdsHi, uint64_t* outIdsLo, size_t maxUUIDs) {
		if (!s_physXScene) return 0;
		physx::PxTransform pose(physx::PxVec3(center.x, center.y, center.z), EulerToPxQuat(orientation));
		physx::PxBoxGeometry geom(halfExtents.x, halfExtents.y, halfExtents.z);
		physx::PxOverlapHit hits[64]; physx::PxOverlapBuffer hitBuffer(hits, 64);

		if (s_physXScene->overlap(geom, pose, hitBuffer)) {
			size_t count = 0;
			for (physx::PxU32 i = 0; i < hitBuffer.nbTouches && count < maxUUIDs; i++) {
				if (hitBuffer.touches[i].actor && hitBuffer.touches[i].actor->userData) {
					Entity entity = { (entt::entity)(uintptr_t)hitBuffer.touches[i].actor->userData, scene };
					if (entity.isValid() && entity.hasComponent<UUIDComponent>()) {
						UUID id = entity.getComponent<UUIDComponent>().id;
						outIdsHi[count] = id.high; outIdsLo[count] = id.low; count++;
					}
				}
			}
			return count;
		}
		return 0;
	}

	size_t PhysicsSystem::overlapSphere(Scene* scene, const Vec3& center, float radius, uint64_t* outIdsHi, uint64_t* outIdsLo, size_t maxUUIDs) {
		if (!s_physXScene) return 0;

		physx::PxSphereGeometry geom(radius);
		physx::PxTransform pose(physx::PxVec3(center.x, center.y, center.z));

		physx::PxOverlapHit hits[64];
		physx::PxOverlapBuffer hitBuffer(hits, 64);

		if (s_physXScene->overlap(geom, pose, hitBuffer)) {
			size_t count = 0;
			for (physx::PxU32 i = 0; i < hitBuffer.nbTouches && count < maxUUIDs; i++) {
				if (hitBuffer.touches[i].actor && hitBuffer.touches[i].actor->userData) {
					entt::entity handle = (entt::entity)(uintptr_t)hitBuffer.touches[i].actor->userData;
					Entity entity = { handle, scene };

					if (entity.isValid() && entity.hasComponent<UUIDComponent>()) {
						UUID id = entity.getComponent<UUIDComponent>().id;
						outIdsHi[count] = id.high;
						outIdsLo[count] = id.low;
						count++;
					}
				}
			}
			return count;
		}
		return 0;
	}

	size_t PhysicsSystem::overlapCapsule(Scene* scene, const Vec3& center, float radius, float halfHeight, const Vec3& orientation, uint64_t* outIdsHi, uint64_t* outIdsLo, size_t maxUUIDs) {
		if (!s_physXScene) return 0;

		physx::PxTransform pose(physx::PxVec3(center.x, center.y, center.z), EulerToPxQuat(orientation));
		physx::PxCapsuleGeometry geom(radius, halfHeight);
		physx::PxOverlapHit hits[64]; physx::PxOverlapBuffer hitBuffer(hits, 64);

		if (s_physXScene->overlap(geom, pose, hitBuffer)) {
			size_t count = 0;
			for (physx::PxU32 i = 0; i < hitBuffer.nbTouches && count < maxUUIDs; i++) {
				if (hitBuffer.touches[i].actor && hitBuffer.touches[i].actor->userData) {
					Entity entity = { (entt::entity)(uintptr_t)hitBuffer.touches[i].actor->userData, scene };
					if (entity.isValid() && entity.hasComponent<UUIDComponent>()) {
						UUID id = entity.getComponent<UUIDComponent>().id;
						outIdsHi[count] = id.high; outIdsLo[count] = id.low; count++;
					}
				}
			}
			return count;
		}
		return 0;
	}

	void PhysicsSystem::createPhysicsActor(Entity entity, Scene* scene, RigidBodyComponent& rb, TransformComponent& transform) {
		PxQuat pxQuat(transform.rotation.x, transform.rotation.y, transform.rotation.z, transform.rotation.w);
		if (pxQuat.magnitudeSquared() < 0.0001) pxQuat = PxQuat(PxIdentity);

		PxTransform physxTransform(PxVec3(transform.position.x, transform.position.y, transform.position.z), pxQuat.getNormalized());

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

		actor->userData = (void*)(uintptr_t)entity.getHandle();

		// -- Setup Box Colliders --
		if (entity.hasComponent<BoxColliderComponent>()) {
			auto& bc = entity.getComponent<BoxColliderComponent>();
			PxMaterial* material = getOrCreatePhysXMaterial(bc.material);
			PxVec3 offset = { bc.offset.x * transform.scale.x, bc.offset.y * transform.scale.y, bc.offset.z * transform.scale.z };
			PxVec3 halfExtents = { std::abs(bc.halfExtents.x * transform.scale.x), std::abs(bc.halfExtents.y * transform.scale.y), std::abs(bc.halfExtents.z * transform.scale.z) };

			if (halfExtents.x < 0.001f) halfExtents.x = 0.001f;
			if (halfExtents.y < 0.001f) halfExtents.y = 0.001f;
			if (halfExtents.z < 0.001f) halfExtents.z = 0.001f;

			PxShape* shape = PxRigidActorExt::createExclusiveShape(*actor, PxBoxGeometry(halfExtents), *material);
			shape->setLocalPose(PxTransform(offset));
			setupShapeFilterData(shape, bc.layer, bc.collisionMask);
			if (bc.isTrigger) {
				shape->setFlag(PxShapeFlag::eSIMULATION_SHAPE, false);
				shape->setFlag(PxShapeFlag::eTRIGGER_SHAPE, true);
			}
			bc.runtimeShape = shape;
		}

		// -- Setup Sphere Colliders --
		if (entity.hasComponent<SphereColliderComponent>()) {
			auto& sc = entity.getComponent<SphereColliderComponent>();
			PxMaterial* material = getOrCreatePhysXMaterial(sc.material);
			float maxScale = std::max(std::abs(transform.scale.x), std::max(std::abs(transform.scale.y), std::abs(transform.scale.z)));
			float geometryRadius = sc.radius * maxScale;
			if (geometryRadius < 0.001f) geometryRadius = 0.001f;

			PxShape* shape = PxRigidActorExt::createExclusiveShape(*actor, PxSphereGeometry(geometryRadius), *material);
			shape->setLocalPose(PxTransform(PxVec3(sc.offset.x * transform.scale.x, sc.offset.y * transform.scale.y, sc.offset.z * transform.scale.z)));
			setupShapeFilterData(shape, sc.layer, sc.collisionMask);
			if (sc.isTrigger) {
				shape->setFlag(PxShapeFlag::eSIMULATION_SHAPE, false);
				shape->setFlag(PxShapeFlag::eTRIGGER_SHAPE, true);
			}
			sc.runtimeShape = shape;
		}

		// -- Setup Capsule Colliders --
		if (entity.hasComponent<CapsuleColliderComponent>()) {
			auto& cc = entity.getComponent<CapsuleColliderComponent>();
			PxMaterial* material = getOrCreatePhysXMaterial(cc.material);
			float scaleXZ = std::max(std::abs(transform.scale.x), std::abs(transform.scale.z));
			float scaledRadius = cc.radius * scaleXZ;
			float scaledHalfHeight = cc.halfHeight * std::abs(transform.scale.y);

			if (scaledRadius < 0.001f) scaledRadius = 0.001f;
			if (scaledHalfHeight < 0.001f) scaledHalfHeight = 0.001f;

			PxShape* shape = PxRigidActorExt::createExclusiveShape(*actor, PxCapsuleGeometry(scaledRadius, scaledHalfHeight), *material);
			PxQuat relativeRotation(PxHalfPi, PxVec3(0.0f, 0.0f, 1.0f));
			PxVec3 offset = { cc.offset.x * transform.scale.x, cc.offset.y * transform.scale.y, cc.offset.z * transform.scale.z };
			shape->setLocalPose(PxTransform(offset, relativeRotation));
			setupShapeFilterData(shape, cc.layer, cc.collisionMask);
			if (cc.isTrigger) {
				shape->setFlag(PxShapeFlag::eSIMULATION_SHAPE, false);
				shape->setFlag(PxShapeFlag::eTRIGGER_SHAPE, true);
			}
			cc.runtimeShape = shape;
		}

		// -- Setup Convex Colliders --
		if (entity.hasComponent<ConvexColliderComponent>()) {
			auto& cc = entity.getComponent<ConvexColliderComponent>();
			PxMaterial* material = getOrCreatePhysXMaterial(cc.material);
			Ref<Mesh> meshAsset = AssetManager::get<Mesh>(cc.collisionMesh);

			if (meshAsset && !meshAsset->getVertices().empty()) {
				PxConvexMeshDesc convexDesc;
				convexDesc.points.count = static_cast<physx::PxU32>(meshAsset->getVertices().size());
				convexDesc.points.stride = sizeof(Axion::Vertex);
				convexDesc.points.data = meshAsset->getVertices().data();
				convexDesc.flags = PxConvexFlag::eCOMPUTE_CONVEX;
				convexDesc.vertexLimit = cc.vertexLimit;

				PxDefaultMemoryOutputStream writeBuffer;
				PxConvexMeshCookingResult::Enum result;
				PxCookingParams params(s_physics->getTolerancesScale());

				if (PxCookConvexMesh(params, convexDesc, writeBuffer, &result)) {
					PxDefaultMemoryInputData readBuffer(writeBuffer.getData(), writeBuffer.getSize());
					PxConvexMesh* convexMesh = s_physics->createConvexMesh(readBuffer);

					PxMeshScale pxScale(PxVec3(transform.scale.x, transform.scale.y, transform.scale.z), PxQuat(PxIdentity));
					PxShape* shape = PxRigidActorExt::createExclusiveShape(*actor, PxConvexMeshGeometry(convexMesh, pxScale), *material);
					setupShapeFilterData(shape, cc.layer, cc.collisionMask);
					if (cc.isTrigger) {
						shape->setFlag(PxShapeFlag::eSIMULATION_SHAPE, false);
						shape->setFlag(PxShapeFlag::eTRIGGER_SHAPE, true);
					}
					cc.runtimeShape = shape;
				}
			}
		}

		// -- Setup Triangle Mesh Colliders (Static Bodies Only) --
		if (entity.hasComponent<TriangleMeshColliderComponent>()) {
			auto& tmc = entity.getComponent<TriangleMeshColliderComponent>();

			if (rb.type != RigidBodyComponent::BodyType::Static) {
				AX_CORE_LOG_WARN("TriangleMeshCollider can only be used with Static RigidBodies.");
			}
			else {
				PxMaterial* material = getOrCreatePhysXMaterial(tmc.material);
				Ref<Mesh> meshAsset = AssetManager::get<Mesh>(tmc.collisionMesh);

				if (meshAsset && !meshAsset->getVertices().empty() && !meshAsset->getIndices().empty()) {
					PxTriangleMeshDesc meshDesc;
					meshDesc.points.count = static_cast<physx::PxU32>(meshAsset->getVertices().size());
					meshDesc.points.stride = sizeof(Axion::Vertex);
					meshDesc.points.data = meshAsset->getVertices().data();
					meshDesc.triangles.count = static_cast<physx::PxU32>(meshAsset->getIndices().size() / 3);
					meshDesc.triangles.stride = 3 * sizeof(uint32_t);
					meshDesc.triangles.data = meshAsset->getIndices().data();

					PxDefaultMemoryOutputStream writeBuffer;
					PxTriangleMeshCookingResult::Enum result;
					PxCookingParams params(s_physics->getTolerancesScale());

					if (PxCookTriangleMesh(params, meshDesc, writeBuffer, &result)) {
						PxDefaultMemoryInputData readBuffer(writeBuffer.getData(), writeBuffer.getSize());
						PxTriangleMesh* triMesh = s_physics->createTriangleMesh(readBuffer);

						PxMeshScale pxScale(PxVec3(transform.scale.x, transform.scale.y, transform.scale.z), PxQuat(PxIdentity));
						PxShape* shape = PxRigidActorExt::createExclusiveShape(*actor, PxTriangleMeshGeometry(triMesh, pxScale), *material);
						setupShapeFilterData(shape, tmc.layer, tmc.collisionMask);
						if (tmc.isTrigger) {
							shape->setFlag(PxShapeFlag::eSIMULATION_SHAPE, false);
							shape->setFlag(PxShapeFlag::eTRIGGER_SHAPE, true);
						}
						tmc.runtimeShape = shape;
					}
				}
			}
		}

		if (rb.type == RigidBodyComponent::BodyType::Dynamic) {
			PxRigidBodyExt::updateMassAndInertia(*(PxRigidDynamic*)actor, rb.mass);
		}

		s_physXScene->addActor(*actor);
		rb.runtimeActor = actor;
	}

}
