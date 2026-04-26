#pragma once

#include "AxionEngine/Source/core/UUID.h"
#include "AxionEngine/Source/core/Math.h"
#include "AxionEngine/Source/physics/PhysicsSystem.h"

namespace Axion {

	enum class ScriptFieldType {
		Float = 0,
		Vector3 = 1
	};

	struct ScriptField {
		std::string name;
		ScriptFieldType type;
	};

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
		void(*transform_getForward)(uint64_t, uint64_t, float*);
		void(*transform_getRight)(uint64_t, uint64_t, float*);
		void(*transform_getUp)(uint64_t, uint64_t, float*);

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
		uint8_t(*physics_raycast)(float*, float*, float, uint64_t*, uint64_t*, float*, float*, float*);

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
		void(*entity_findEntityByName)(const char*, uint64_t*, uint64_t*);
		void*(*entity_getScriptInstance)(uint64_t, uint64_t);
		void(*entity_emitParticles)(uint64_t, uint64_t, int);

		// -- ANIMATION --
		void(*animation_play)(uint64_t, uint64_t);
		void(*animation_stop)(uint64_t, uint64_t);
		uint8_t(*animation_isPlaying)(uint64_t, uint64_t);

		// -- REFLECTION --
		void(*script_registerField)(const char*, const char*, int);

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
		static void loadAppAssembly(const std::filesystem::path& filePath);
		static bool compileAppAssembly(const std::filesystem::path& csprojPath);

		// -- Metadata reflection --
		static void registerScriptField(const std::string& className, const std::string& fieldName, ScriptFieldType type);
		static const std::vector<ScriptField>& getScriptFields(const std::string& className);
		static float getFieldValueFloat(void* gcHandle, const std::string& fieldName);
		static void setFieldValueFloat(void* gcHandle, const std::string& fieldName, float value);
		static Vec3 getFieldValueVector3(void* gcHandle, const std::string& fieldName);
		static void setFieldValueVector3(void* gcHandle, const std::string& fieldName, const Vec3& value);

	private:

		static Scene* s_sceneContext;
		static std::unordered_map<std::string, std::vector<ScriptField>> s_scriptMetadata;

		static bool loadHostFxr();

	};

}
