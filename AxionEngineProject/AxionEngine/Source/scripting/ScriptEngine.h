#pragma once

#include "AxionEngine/Source/core/UUID.h"
#include "AxionEngine/Source/core/Math.h"
#include "AxionEngine/Source/physics/PhysicsSystem.h"

namespace Axion {

	struct ScriptAPI {
		// -- INPUT --
		uint8_t(*input_isKeyPressed)(uint16_t);
		uint8_t(*input_isMouseButtonPressed)(uint16_t);
		void(*input_getMousePosition)(float*, float*);

		// -- TRANSFORM --
		void(*transform_getPosition)(uint64_t, uint64_t, float*);
		void(*transform_setPosition)(uint64_t, uint64_t, float*);
		void(*transform_getRotation)(uint64_t, uint64_t, float*);
		void(*transform_setRotation)(uint64_t, uint64_t, float*);
		void(*transform_getScale)(uint64_t, uint64_t, float*);
		void(*transform_setScale)(uint64_t, uint64_t, float*);

		// -- PHYSICS --
		void(*rigidbody_addForce)(uint64_t, uint64_t, float*, int);
		void(*rigidbody_addTorque)(uint64_t, uint64_t, float*, int);
		void(*rigidbody_addRadialImpulse)(uint64_t, uint64_t, float*, float, float);
		void(*rigidbody_getLinearVelocity)(uint64_t, uint64_t, float*);
		void(*rigidbody_setLinearVelocity)(uint64_t, uint64_t, float*);
		void(*rigidbody_getAngularVelocity)(uint64_t, uint64_t, float*);
		void(*rigidbody_setAngularVelocity)(uint64_t, uint64_t, float*);
		float(*rigidbody_getMass)(uint64_t, uint64_t);
		void(*rigidbody_setMass)(uint64_t, uint64_t, float);

		// -- AUDIO --
		void(*audio_play)(uint64_t, uint64_t);
		void(*audio_stop)(uint64_t, uint64_t);
		void(*audio_setVolume)(uint64_t, uint64_t, float);
		float(*audio_getVolume)(uint64_t, uint64_t);
		void(*audio_setPitch)(uint64_t, uint64_t, float);
		float(*audio_getPitch)(uint64_t, uint64_t);

		// -- ENTITY --
		void(*entity_instantiate)(const char*, uint64_t*, uint64_t*);
		void(*entity_instantiatePrefab)(const char*, uint64_t*, uint64_t*);
		void(*entity_destroy)(uint64_t, uint64_t);
		void(*entity_addComponent)(uint64_t, uint64_t, int);
		void(*entity_addScript)(uint64_t, uint64_t, const char*);
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
		static void updateTime(float deltaTime);

	private:

		static Scene* s_sceneContext;

		static bool loadHostFxr();

	};

}
