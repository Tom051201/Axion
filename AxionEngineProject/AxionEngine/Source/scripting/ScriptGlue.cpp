#include "axpch.h"
#include "ScriptGlue.h"

#include "AxionEngine/Source/input/Input.h"
#include "AxionEngine/Source/scene/Entity.h"
#include "AxionEngine/Source/physics/Physics.h"

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
				entity.getComponent<TransformComponent>().position = Vec3(inPos[0], inPos[1], inPos[2]);
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
				entity.getComponent<TransformComponent>().setEulerAngles(Vec3(inRot[0], inRot[1], inRot[2]));
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

	}



	// -- Register macro --
	#define REGISTER_API(apiStruct, functionName) apiStruct.functionName = InternalCalls::functionName



	// -- Binding function --
	void ScriptGlue::registerComponents(ScriptAPI& apiStruct) {

		REGISTER_API(apiStruct, input_isKeyPressed);
		REGISTER_API(apiStruct, input_isMouseButtonPressed);
		REGISTER_API(apiStruct, input_getMousePosition);

		REGISTER_API(apiStruct, transform_getPosition);
		REGISTER_API(apiStruct, transform_setPosition);
		REGISTER_API(apiStruct, transform_getRotation);
		REGISTER_API(apiStruct, transform_setRotation);
		REGISTER_API(apiStruct, transform_getScale);
		REGISTER_API(apiStruct, transform_setScale);

		REGISTER_API(apiStruct, rigidbody_addForce);
		REGISTER_API(apiStruct, rigidbody_addTorque);
		REGISTER_API(apiStruct, rigidbody_getLinearVelocity);
		REGISTER_API(apiStruct, rigidbody_setLinearVelocity);
		REGISTER_API(apiStruct, rigidbody_getMass);
		REGISTER_API(apiStruct, rigidbody_setMass);

		AX_CORE_LOG_TRACE("[ScriptGlue] All internal C++ functions registered to C#!");
	}

}
