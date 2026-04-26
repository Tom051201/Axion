#include "axpch.h"
#include "ScriptGlue.h"

#include "AxionEngine/Source/input/Input.h"
#include "AxionEngine/Source/scene/Entity.h"
#include "AxionEngine/Source/physics/Physics.h"
#include "AxionEngine/Source/core/AssetVersions.h"
#include "AxionEngine/Source/core/AssetManager.h"
#include "AxionEngine/Source/scene/SceneSerializer.h"
#include "AxionEngine/Source/scene/Prefab.h"

#include "AxionEngine/Vendor/yaml-cpp/include/yaml-cpp/yaml.h"

namespace Axion {

	namespace InternalCalls {

		// -- Helper function --
		static Entity getEntityByUUID(uint64_t hi, uint64_t lo) {
			Scene* scene = ScriptEngine::getSceneContext();
			if (!scene) return {};
			return scene->getEntityByUUID(UUID(hi, lo));
		}

		// -- CPP function pointers --

		// -- INPUT --
		extern "C" uint8_t input_isKeyPressed(uint16_t keyCode) {
			return Input::isKeyPressed(static_cast<KeyCode>(keyCode)) ? 1 : 0;
		}

		extern "C" uint8_t input_isMouseButtonPressed(uint16_t button) {
			return Input::isMouseButtonPressed(static_cast<MouseButton>(button)) ? 1 : 0;
		}

		extern "C" void input_getMousePosition(float* outX, float* outY) {
			auto [x, y] = Input::getMousePosition();
			*outX = x;
			*outY = y;
		}


		// -- TRANSFORM --
		extern "C" void transform_getPosition(uint64_t uuidHi, uint64_t uuidLo, float* outPos) {
			Entity entity = getEntityByUUID(uuidHi, uuidLo);
			if (entity.isValid() && entity.hasComponent<TransformComponent>()) {
				Vec3 pos = entity.getComponent<TransformComponent>().position;
				outPos[0] = pos.x;
				outPos[1] = pos.y;
				outPos[2] = pos.z;
			}
		}

		extern "C" void transform_setPosition(uint64_t uuidHi, uint64_t uuidLo, float* inPos) {
			Entity entity = getEntityByUUID(uuidHi, uuidLo);
			if (entity.isValid() && entity.hasComponent<TransformComponent>()) {
				Vec3 newPos = Vec3(inPos[0], inPos[1], inPos[2]);
				entity.getComponent<TransformComponent>().position = newPos;
				if (entity.hasComponent<RigidBodyComponent>()) {
					Physics::setPosition(entity, newPos);
				}
			}
		}

		extern "C" void transform_getRotation(uint64_t uuidHi, uint64_t uuidLo, float* outRot) {
			Entity entity = getEntityByUUID(uuidHi, uuidLo);
			if (entity.isValid() && entity.hasComponent<TransformComponent>()) {
				Vec3 rot = entity.getComponent<TransformComponent>().getEulerAngles();
				outRot[0] = rot.x;
				outRot[1] = rot.y;
				outRot[2] = rot.z;
			}
		}

		extern "C" void transform_setRotation(uint64_t uuidHi, uint64_t uuidLo, float* inRot) {
			Entity entity = getEntityByUUID(uuidHi, uuidLo);
			if (entity.isValid() && entity.hasComponent<TransformComponent>()) {
				Vec3 newRot = Vec3(inRot[0], inRot[1], inRot[2]);
				entity.getComponent<TransformComponent>().setEulerAngles(newRot);
				if (entity.hasComponent<RigidBodyComponent>()) {
					Physics::setRotation(entity, entity.getComponent<TransformComponent>().rotation);
				}
			}
		}

		extern "C" void transform_getScale(uint64_t uuidHi, uint64_t uuidLo, float* outScale) {
			Entity entity = getEntityByUUID(uuidHi, uuidLo);
			if (entity.isValid() && entity.hasComponent<TransformComponent>()) {
				Vec3 scale = entity.getComponent<TransformComponent>().scale;
				outScale[0] = scale.x;
				outScale[1] = scale.y;
				outScale[2] = scale.z;
			}
		}

		extern "C" void transform_setScale(uint64_t uuidHi, uint64_t uuidLo, float* inScale) {
			Entity entity = getEntityByUUID(uuidHi, uuidLo);
			if (entity.isValid() && entity.hasComponent<TransformComponent>()) {
				entity.getComponent<TransformComponent>().scale = Vec3(inScale[0], inScale[1], inScale[2]);
			}
		}

		extern "C" void transform_getForward(uint64_t uuidHi, uint64_t uuidLo, float* outForward) {
			Entity entity = getEntityByUUID(uuidHi, uuidLo);
			Scene* scene = ScriptEngine::getSceneContext();
			if (entity.isValid() && scene) {
				Mat4 transform = scene->getWorldTransform(entity);
				Vec4 forward = transform * Vec4(0.0f, 0.0f, 1.0f, 0.0f);
				Vec3 result = { forward.x, forward.y, forward.z };
				result.normalized();

				outForward[0] = result.x;
				outForward[1] = result.y;
				outForward[2] = result.z;
			}
		}

		extern "C" void transform_getRight(uint64_t uuidHi, uint64_t uuidLo, float* outRight) {
			Entity entity = getEntityByUUID(uuidHi, uuidLo);
			Scene* scene = ScriptEngine::getSceneContext();
			if (entity.isValid() && scene) {
				Mat4 transform = scene->getWorldTransform(entity);
				Vec4 right = transform * Vec4(1.0f, 0.0f, 0.0f, 0.0f);
				Vec3 result = { right.x, right.y, right.z };
				result.normalized();

				outRight[0] = result.x;
				outRight[1] = result.y;
				outRight[2] = result.z;
			}
		}

		extern "C" void transform_getUp(uint64_t uuidHi, uint64_t uuidLo, float* outUp) {
			Entity entity = getEntityByUUID(uuidHi, uuidLo);
			Scene* scene = ScriptEngine::getSceneContext();
			if (entity.isValid() && scene) {
				Mat4 transform = scene->getWorldTransform(entity);
				Vec4 up = transform * Vec4(0.0f, 1.0f, 0.0f, 0.0f);
				Vec3 result = { up.x, up.y, up.z };
				result.normalized();

				outUp[0] = result.x;
				outUp[1] = result.y;
				outUp[2] = result.z;
			}
		}


		// -- PHYSICS --
		extern "C" void rigidbody_addForce(uint64_t uuidHi, uint64_t uuidLo, float* force, int mode) {
			Entity entity = getEntityByUUID(uuidHi, uuidLo);
			if (entity.isValid()) {
				Physics::addForce(entity, Vec3(force[0], force[1], force[2]), static_cast<ForceMode>(mode));
			}
		}

		extern "C" void rigidbody_addTorque(uint64_t uuidHi, uint64_t uuidLo, float* torque, int mode) {
			Entity entity = getEntityByUUID(uuidHi, uuidLo);
			if (entity.isValid()) {
				Physics::addTorque(entity, Vec3(torque[0], torque[1], torque[2]), static_cast<ForceMode>(mode));
			}
		}

		extern "C" void rigidbody_addRadialImpulse(uint64_t uuidHi, uint64_t uuidLo, float* origin, float radius, float strength) {
			Entity entity = getEntityByUUID(uuidHi, uuidLo);
			if (entity.isValid()) {
				Physics::addRadialImpulse(entity, Vec3(origin[0], origin[1], origin[2]), radius, strength);
			}
		}

		extern "C" void rigidbody_getLinearVelocity(uint64_t uuidHi, uint64_t uuidLo, float* outVel) {
			Entity entity = getEntityByUUID(uuidHi, uuidLo);
			if (entity.isValid()) {
				Vec3 vel = Physics::getLinearVelocity(entity);
				outVel[0] = vel.x;
				outVel[1] = vel.y;
				outVel[2] = vel.z;
			}
		}

		extern "C" void rigidbody_setLinearVelocity(uint64_t uuidHi, uint64_t uuidLo, float* inVel) {
			Entity entity = getEntityByUUID(uuidHi, uuidLo);
			if (entity.isValid()) {
				Physics::setLinearVelocity(entity, Vec3(inVel[0], inVel[1], inVel[2]));
			}
		}

		extern "C" void rigidbody_getAngularVelocity(uint64_t uuidHi, uint64_t uuidLo, float* outVel) {
			Entity entity = getEntityByUUID(uuidHi, uuidLo);
			if (entity.isValid()) {
				Vec3 vel = Physics::getAngularVelocity(entity);
				outVel[0] = vel.x;
				outVel[1] = vel.y;
				outVel[2] = vel.z;
			}
		}

		extern "C" void rigidbody_setAngularVelocity(uint64_t uuidHi, uint64_t uuidLo, float* inVel) {
			Entity entity = getEntityByUUID(uuidHi, uuidLo);
			if (entity.isValid()) {
				Physics::setAngularVelocity(entity, Vec3(inVel[0], inVel[1], inVel[2]));
			}
		}

		extern "C" float rigidbody_getMass(uint64_t uuidHi, uint64_t uuidLo) {
			Entity entity = getEntityByUUID(uuidHi, uuidLo);
			if (entity.isValid()) {
				return Physics::getMass(entity);
			}
			return 0.0f;
		}

		extern "C" void rigidbody_setMass(uint64_t uuidHi, uint64_t uuidLo, float mass) {
			Entity entity = getEntityByUUID(uuidHi, uuidLo);
			if (entity.isValid()) {
				Physics::setMass(entity, mass);
			}
		}

		extern "C" uint8_t physics_raycast(float* origin, float* direction, float maxDistance, uint64_t* outIdHi, uint64_t* outIdLo, float* outPos, float* outNormal, float* outDistance) {
			Scene* scene = ScriptEngine::getSceneContext();
			if (!scene) return 0;

			RaycastHit hit;
			if (PhysicsSystem::raycast(scene, Vec3(origin[0], origin[1], origin[2]), Vec3(direction[0], direction[1], direction[2]), maxDistance, &hit)) {
				if (hit.entity.isValid()) {
					UUID id = hit.entity.getComponent<UUIDComponent>().id;
					*outIdHi = id.high;
					*outIdLo = id.low;
				}
				else {
					*outIdHi = 0;
					*outIdLo = 0;
				}

				outPos[0] = hit.position.x;
				outPos[1] = hit.position.y;
				outPos[2] = hit.position.z;
				outNormal[0] = hit.normal.x;
				outNormal[1] = hit.normal.y;
				outNormal[2] = hit.normal.z;
				*outDistance = hit.distance;

				return 1;
			}
			return 0;
		}


		// -- AUDIO --
		extern "C" void audio_play(uint64_t uuidHi, uint64_t uuidLo) {
			Entity entity = getEntityByUUID(uuidHi, uuidLo);
			if (entity.isValid() && entity.hasComponent<AudioComponent>()) {
				auto& ac = entity.getComponent<AudioComponent>();
				if (ac.audio) ac.audio->play();
			}
		}

		extern "C" void audio_stop(uint64_t uuidHi, uint64_t uuidLo) {
			Entity entity = getEntityByUUID(uuidHi, uuidLo);
			if (entity.isValid() && entity.hasComponent<AudioComponent>()) {
				auto& ac = entity.getComponent<AudioComponent>();
				if (ac.audio) ac.audio->stop();
			}
		}

		extern "C" void audio_setVolume(uint64_t uuidHi, uint64_t uuidLo, float vol) {
			Entity entity = getEntityByUUID(uuidHi, uuidLo);
			if (entity.isValid() && entity.hasComponent<AudioComponent>()) {
				auto& ac = entity.getComponent<AudioComponent>();
				if (ac.audio) ac.audio->setVolume(vol);
			}
		}

		extern "C" float audio_getVolume(uint64_t uuidHi, uint64_t uuidLo) {
			Entity entity = getEntityByUUID(uuidHi, uuidLo);
			if (entity.isValid() && entity.hasComponent<AudioComponent>()) {
				auto& ac = entity.getComponent<AudioComponent>();
				if (ac.audio) return ac.audio->getVolume();
			}
			return 0.0f;
		}

		extern "C" void audio_setPitch(uint64_t uuidHi, uint64_t uuidLo, float pitch) {
			Entity entity = getEntityByUUID(uuidHi, uuidLo);
			if (entity.isValid() && entity.hasComponent<AudioComponent>()) {
				auto& ac = entity.getComponent<AudioComponent>();
				if (ac.audio) ac.audio->setPitch(pitch);
			}
		}

		extern "C" float audio_getPitch(uint64_t uuidHi, uint64_t uuidLo) {
			Entity entity = getEntityByUUID(uuidHi, uuidLo);
			if (entity.isValid() && entity.hasComponent<AudioComponent>()) {
				auto& ac = entity.getComponent<AudioComponent>();
				if (ac.audio) return ac.audio->getPitch();
			}
			return 0.0f;
		}


		// -- ENTITY --
		extern "C" void entity_instantiate(const char* name, uint64_t* outUuidHi, uint64_t* outUuidLo) {
			Scene* scene = ScriptEngine::getSceneContext();
			if (!scene) return;

			std::string entityName = name ? name : "Instantiated Entity";

			Entity newEntity = scene->createEntity(entityName);
			UUID newID = newEntity.getComponent<UUIDComponent>().id;

			*outUuidHi = newID.high;
			*outUuidLo = newID.low;
		}

		extern "C" void entity_instantiatePrefab(const char* filePath, uint64_t* outUuidHi, uint64_t* outUuidLo) {
			Scene* scene = ScriptEngine::getSceneContext();
			if (!scene || !filePath) return;

			std::string pathStr = filePath;
			std::string absPath = AssetManager::getAbsolute(pathStr).string();

			UUID prefabUUID = AssetManager::getAssetUUID(absPath);
			if (prefabUUID.isValid()) {
				AssetHandle<Prefab> handle = AssetManager::load<Prefab>(prefabUUID);
				Ref<Prefab> prefab = AssetManager::get<Prefab>(handle);

				if (prefab) {
					Entity newEntity;

					// -- Load Binary if in Runtime Mode --
					if (prefab->isBinary()) {
						const std::vector<uint8_t>& binaryData = prefab->getBinaryData();

						std::string dataStr(binaryData.begin(), binaryData.end());
						std::istringstream in(dataStr);

						std::vector<std::pair<Entity, UUID>> relationshipsToBuild;
						newEntity = SceneSerializer::deserializeEntityBinary(scene, in, true, relationshipsToBuild, ASSET_VERSION_SCENE);

						for (auto& pair : relationshipsToBuild) {
							Entity child = pair.first;
							Entity parent = scene->getEntityByUUID(pair.second);
							if (parent.isValid()) {
								child.setParent(parent);
							}
						}
					}
					// -- Load YAML
					else {
						YAML::Node node = prefab->getEntityNode();
						Entity newEntity = SceneSerializer::deserializeEntityNode(scene, node, true);
					}

					if (newEntity.isValid()) {
						UUID newID = newEntity.getComponent<UUIDComponent>().id;
						*outUuidHi = newID.high;
						*outUuidLo = newID.low;

						AX_CORE_LOG_TRACE("Instantiated Prefab successfully!");
						return;
					}
				}
			}

			AX_CORE_LOG_ERROR("Failed to instantiate Prefab: {}", pathStr);
			*outUuidHi = 0;
			*outUuidLo = 0;
		}

		extern "C" void entity_destroy(uint64_t uuidHi, uint64_t uuidLo) {
			Entity entity = getEntityByUUID(uuidHi, uuidLo);
			if (entity.isValid()) {
				ScriptEngine::getSceneContext()->destroyEntity(entity);
			}
		}

		extern "C" void entity_addComponent(uint64_t uuidHi, uint64_t uuidLo, int type) {
			Entity entity = getEntityByUUID(uuidHi, uuidLo);
			if (!entity.isValid()) return;

			switch (type) {

				case 0: { if (!entity.hasComponent<RigidBodyComponent>()) entity.addComponent<RigidBodyComponent>(); break; }
				case 1: { if (!entity.hasComponent<BoxColliderComponent>()) entity.addComponent<BoxColliderComponent>(); break; }
				case 2: { if (!entity.hasComponent<SphereColliderComponent>()) entity.addComponent<SphereColliderComponent>(); break; }
				case 3: { if (!entity.hasComponent<CapsuleColliderComponent>()) entity.addComponent<CapsuleColliderComponent>(); break; }
				case 4: { if (!entity.hasComponent<AudioComponent>()) entity.addComponent<AudioComponent>(); break; }
				case 5: { if (!entity.hasComponent<ParticleSystemComponent>()) entity.addComponent<ParticleSystemComponent>(); break; }

			}
		}

		extern "C" void entity_addScript(uint64_t uuidHi, uint64_t uuidLo, const char* scriptName) {
			Entity entity = getEntityByUUID(uuidHi, uuidLo);
			if (entity.isValid() && !entity.hasComponent<ScriptComponent>()) {
				std::string nameStr = scriptName ? scriptName : "";
				if (!nameStr.empty()) {
					entity.addComponent<ScriptComponent>(nameStr);
				}
			}
		}

		extern "C" void entity_findEntityByName(const char* name, uint64_t* outUuidHi, uint64_t* outUuidLo) {
			Scene* scene = ScriptEngine::getSceneContext();
			if (!scene || !name) {
				*outUuidHi = 0;
				*outUuidLo = 0;
				return;
			}

			std::string searchName = name;

			auto view = scene->getRegistry().view<TagComponent>();
			for (auto e : view) {
				if (view.get<TagComponent>(e).tag == searchName) {
					Entity entity = { e, scene };
					UUID id = entity.getComponent<UUIDComponent>().id;
					*outUuidHi = id.high;
					*outUuidLo = id.low;
					return;
				}
			}

			// Not found
			*outUuidHi = 0;
			*outUuidLo = 0;
		}

		extern "C" void* entity_getScriptInstance(uint64_t uuidHi, uint64_t uuidLo) {
			Entity entity = getEntityByUUID(uuidHi, uuidLo);
			if (entity.isValid() && entity.hasComponent<ScriptComponent>()) {
				return entity.getComponent<ScriptComponent>().gcHandle;
			}
			return nullptr;
		}

		extern "C" void entity_emitParticles(uint64_t uuidHi, uint64_t uuidLo, int count) {
			Entity entity = getEntityByUUID(uuidHi, uuidLo);
			if (entity.isValid() && entity.hasComponent<ParticleSystemComponent>()) {
				auto& psc = entity.getComponent<ParticleSystemComponent>();
				auto& transform = entity.getComponent<TransformComponent>();
				for (int i = 0; i < count; i++) {
					ParticleSystem::emitParticle(psc, transform.position);
				}
			}
		}


		// -- ANIMATION --
		extern "C" void animation_play(uint64_t uuidHi, uint64_t uuidLo) {
			Entity entity = getEntityByUUID(uuidHi, uuidLo);
			if (entity.isValid() && entity.hasComponent<AnimatorComponent>()) {
				entity.getComponent<AnimatorComponent>().isPlaying = true;
			}
		}

		extern "C" void animation_stop(uint64_t uuidHi, uint64_t uuidLo) {
			Entity entity = getEntityByUUID(uuidHi, uuidLo);
			if (entity.isValid() && entity.hasComponent<AnimatorComponent>()) {
				entity.getComponent<AnimatorComponent>().isPlaying = false;
			}
		}

		extern "C" uint8_t animation_isPlaying(uint64_t uuidHi, uint64_t uuidLo) {
			Entity entity = getEntityByUUID(uuidHi, uuidLo);
			if (entity.isValid() && entity.hasComponent<AnimatorComponent>()) {
				return entity.getComponent<AnimatorComponent>().isPlaying ? 1 : 0;
			}
			return 0;
		}


		// -- REFLECTION --
		extern "C" void script_registerField(const char* className, const char* fieldName, int type) {
			ScriptEngine::registerScriptField(className, fieldName, static_cast<ScriptFieldType>(type));
		}

	}



	// -- Register macro --
	#define REGISTER_API(apiStruct, functionName) apiStruct.functionName = InternalCalls::functionName



	// -- Binding function --
	void ScriptGlue::registerComponents(ScriptAPI& apiStruct) {

		// -- INPUT --
		REGISTER_API(apiStruct, input_isKeyPressed);
		REGISTER_API(apiStruct, input_isMouseButtonPressed);
		REGISTER_API(apiStruct, input_getMousePosition);

		// -- TRANSFORM --
		REGISTER_API(apiStruct, transform_getPosition);
		REGISTER_API(apiStruct, transform_setPosition);
		REGISTER_API(apiStruct, transform_getRotation);
		REGISTER_API(apiStruct, transform_setRotation);
		REGISTER_API(apiStruct, transform_getScale);
		REGISTER_API(apiStruct, transform_setScale);
		REGISTER_API(apiStruct, transform_getForward);
		REGISTER_API(apiStruct, transform_getRight);
		REGISTER_API(apiStruct, transform_getUp);

		// -- PHYSICS --
		REGISTER_API(apiStruct, rigidbody_addForce);
		REGISTER_API(apiStruct, rigidbody_addTorque);
		REGISTER_API(apiStruct, rigidbody_addRadialImpulse);
		REGISTER_API(apiStruct, rigidbody_getLinearVelocity);
		REGISTER_API(apiStruct, rigidbody_setLinearVelocity);
		REGISTER_API(apiStruct, rigidbody_getAngularVelocity);
		REGISTER_API(apiStruct, rigidbody_setAngularVelocity);
		REGISTER_API(apiStruct, rigidbody_getMass);
		REGISTER_API(apiStruct, rigidbody_setMass);
		REGISTER_API(apiStruct, physics_raycast);

		// -- AUDIO --
		REGISTER_API(apiStruct, audio_play);
		REGISTER_API(apiStruct, audio_stop);
		REGISTER_API(apiStruct, audio_getVolume);
		REGISTER_API(apiStruct, audio_setVolume);
		REGISTER_API(apiStruct, audio_getPitch);
		REGISTER_API(apiStruct, audio_setPitch);

		// -- ENTITY --
		REGISTER_API(apiStruct, entity_instantiate);
		REGISTER_API(apiStruct, entity_instantiatePrefab);
		REGISTER_API(apiStruct, entity_destroy);
		REGISTER_API(apiStruct, entity_addComponent);
		REGISTER_API(apiStruct, entity_addScript);
		REGISTER_API(apiStruct, entity_findEntityByName);
		REGISTER_API(apiStruct, entity_getScriptInstance);
		REGISTER_API(apiStruct, entity_emitParticles);

		// -- ANIMATION --
		REGISTER_API(apiStruct, animation_play);
		REGISTER_API(apiStruct, animation_stop);
		REGISTER_API(apiStruct, animation_isPlaying);

		// -- REFLECTION --
		REGISTER_API(apiStruct, script_registerField);


		AX_CORE_LOG_TRACE("[ScriptGlue] All internal C++ functions registered to C#!");
	}

}
