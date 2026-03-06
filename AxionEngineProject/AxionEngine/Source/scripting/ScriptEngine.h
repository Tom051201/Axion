#pragma once

#include "AxionEngine/Source/core/UUID.h"
#include "AxionEngine/Source/core/Math.h"
#include "AxionEngine/Source/physics/PhysicsSystem.h"

namespace Axion {

	struct ScriptAPI {
		uint8_t(*input_isKeyPressed)(uint16_t);
		uint8_t(*input_isMouseButtonPressed)(uint16_t);
		void(*input_getMousePosition)(float*, float*);

		void(*transform_getPosition)(uint64_t, uint64_t, float*);
		void(*transform_setPosition)(uint64_t, uint64_t, float*);
		void(*transform_getRotation)(uint64_t, uint64_t, float*);
		void(*transform_setRotation)(uint64_t, uint64_t, float*);
		void(*transform_getScale)(uint64_t, uint64_t, float*);
		void(*transform_setScale)(uint64_t, uint64_t, float*);

		void(*rigidbody_addForce)(uint64_t, uint64_t, float*, int);
		void(*rigidbody_addTorque)(uint64_t, uint64_t, float*, int);
		void(*rigidbody_getLinearVelocity)(uint64_t, uint64_t, float*);
		void(*rigidbody_setLinearVelocity)(uint64_t, uint64_t, float*);
		float(*rigidbody_getMass)(uint64_t, uint64_t);
		void(*rigidbody_setMass)(uint64_t, uint64_t, float);
	};



	class Scene;

	class ScriptEngine {
	public:

		static void initialize();
		static void shutdown();

		static void setSceneContext(Scene* scene);
		static Scene* getSceneContext();

		static void* createEntityScript(UUID entityID, const char* scriptName);
		static void destroyEntityScript(void* gcHandle);
		static void updateEntityScript(void* gcHandle, float timestep);
		static void onCollisionEnter(void* gcHandle, Collision& collision);
		static void onCollisionExit(void* gcHandle, Collision& collision);

	private:

		static Scene* s_sceneContext;

		static bool loadHostFxr();

	};

}
