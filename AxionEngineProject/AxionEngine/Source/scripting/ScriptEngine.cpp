#include "axpch.h"
#include "ScriptEngine.h"

#include "AxionEngine/Source/scripting/ScriptGlue.h"
#include "AxionEngine/Source/scene/Scene.h"

#include <nethost.h>
#include <coreclr_delegates.h>
#include <hostfxr.h>

namespace Axion {
	
	Scene* ScriptEngine::s_sceneContext = nullptr;

	static hostfxr_initialize_for_runtime_config_fn s_initFnptr = nullptr;
	static hostfxr_get_runtime_delegate_fn s_getDelegateFnptr = nullptr;
	static hostfxr_close_fn s_closeFnptr = nullptr;

	static hostfxr_handle s_context = nullptr;
	static load_assembly_and_get_function_pointer_fn s_loadAssemblyAndGetFuncPtr = nullptr;

	static ScriptAPI s_api;

	struct CSharpCollision {
		uint64_t entityIdHigh;
		uint64_t entityIdLow;
		float contactPoint[3];
		float contactNormal[3];
		float impulse[3];
	};

	typedef void*(*createScriptFunc)(uint64_t, uint64_t, const char*);
	typedef void(*destroyScriptFunc)(void*);
	typedef void(*updateScriptFunc)(void*, float);
	typedef void(*collisionFunc)(void*, CSharpCollision*);
	typedef void (*updateTimeFunc)(float);

	static createScriptFunc s_createEntityScriptFunc = nullptr;
	static destroyScriptFunc s_destroyEntityScriptFunc = nullptr;
	static updateScriptFunc s_updateEntityScriptFunc = nullptr;
	static collisionFunc s_onCollisionEnterFunc = nullptr;
	static collisionFunc s_onCollisionExitFunc = nullptr;
	static updateTimeFunc s_updateTimeFunc = nullptr;

	void ScriptEngine::initialize() {
		bool loadHostSuccess = loadHostFxr();
		if (!loadHostSuccess) {
			AX_CORE_LOG_ERROR("[ScriptEngine] Failed to load .NET HostFXR");
		}

		AX_CORE_LOG_INFO("[ScriptEngine] Successfully loaded .NET HostFXR");

		// -- Find exe file --
		wchar_t exePath[MAX_PATH];
		GetModuleFileNameW(nullptr, exePath, MAX_PATH);
		std::filesystem::path appDir = std::filesystem::path(exePath).parent_path();

		std::wstring configPathStr = (appDir / L"AxionScriptCore.runtimeconfig.json").wstring();
		std::wstring assemblyPathStr = (appDir / L"AxionScriptCore.dll").wstring();

		const wchar_t* configPath = configPathStr.c_str();
		const wchar_t* assemblyPath = assemblyPathStr.c_str();

		int rc = s_initFnptr(configPath, nullptr, &s_context);
		if (rc != 0 || s_context == nullptr) {
			AX_CORE_LOG_ERROR("[ScriptEngine] Failed to initialize .NET runtime. Error code: {0:x}", rc);
			s_closeFnptr(s_context);
			return;
		}

		rc = s_getDelegateFnptr(s_context, hdt_load_assembly_and_get_function_pointer, (void**)&s_loadAssemblyAndGetFuncPtr);
		if (rc != 0 || s_loadAssemblyAndGetFuncPtr == nullptr) {
			AX_CORE_LOG_ERROR("[ScriptEngine] Failed to get load delegate. Error code: {0:x}", rc);
			return;
		}

		const wchar_t* typeName = L"AxionScriptCore.CoreAPI, AxionScriptCore";	// Namespace.Class, AssemblyName
		const wchar_t* methodName = L"Initialize";								// Matches the [UnmanagedCallersOnly] EntryPoint name

		typedef void(*initFunc)(ScriptAPI*);
		initFunc initEngineFunc = nullptr;

		rc = s_loadAssemblyAndGetFuncPtr(assemblyPath, typeName, methodName, UNMANAGEDCALLERSONLY_METHOD, nullptr, (void**)&initEngineFunc);
		if (rc != 0 || initEngineFunc == nullptr) {
			AX_CORE_LOG_ERROR("[ScriptEngine] Failed to load C# method. Error code: {0:x}", rc);
			return;
		}

		AX_CORE_LOG_INFO("[ScriptEngine] Passing API struct to C#...");

		ScriptGlue::registerComponents(s_api);

		initEngineFunc(&s_api);

		AX_CORE_LOG_INFO("[ScriptEngine] Loading ScriptManager functions...");

		const wchar_t* managerTypeName = L"AxionScriptCore.ScriptManager, AxionScriptCore";

		// -- Load createEntityScript --
		rc = s_loadAssemblyAndGetFuncPtr(assemblyPath, managerTypeName, L"CreateEntityScript", UNMANAGEDCALLERSONLY_METHOD, nullptr, (void**)&s_createEntityScriptFunc);
		if (rc != 0 || s_createEntityScriptFunc == nullptr) {
			AX_CORE_LOG_ERROR("[ScriptEngine] Failed to load CreateEntityScript. Error code: {0:x}", rc);
			return;
		}

		// -- Load destroyEntityScript --
		rc = s_loadAssemblyAndGetFuncPtr(assemblyPath, managerTypeName, L"DestroyEntityScript", UNMANAGEDCALLERSONLY_METHOD, nullptr, (void**)&s_destroyEntityScriptFunc);
		if (rc != 0 || s_destroyEntityScriptFunc == nullptr) {
			AX_CORE_LOG_ERROR("[ScriptEngine] Failed to load DestroyEntityScript.");
			return;
		}

		// -- Load updateEntityScript --
		rc = s_loadAssemblyAndGetFuncPtr(assemblyPath, managerTypeName, L"UpdateEntityScript", UNMANAGEDCALLERSONLY_METHOD, nullptr, (void**)&s_updateEntityScriptFunc);
		if (rc != 0 || s_updateEntityScriptFunc == nullptr) {
			AX_CORE_LOG_ERROR("[ScriptEngine] Failed to load UpdateEntityScript. Error code: {0:x}", rc);
			return;
		}

		// -- Load OnCollisionEnter --
		rc = s_loadAssemblyAndGetFuncPtr(assemblyPath, managerTypeName, L"OnCollisionEnterScript", UNMANAGEDCALLERSONLY_METHOD, nullptr, (void**)&s_onCollisionEnterFunc);
		if (rc != 0 || s_onCollisionEnterFunc == nullptr) {
			AX_CORE_LOG_ERROR("[ScriptEngine] Failed to load OnCollisionEnterScript.");
		}

		// -- Load OnCollisionExit --
		rc = s_loadAssemblyAndGetFuncPtr(assemblyPath, managerTypeName, L"OnCollisionExitScript", UNMANAGEDCALLERSONLY_METHOD, nullptr, (void**)&s_onCollisionExitFunc);
		if (rc != 0 || s_onCollisionExitFunc == nullptr) {
			AX_CORE_LOG_ERROR("[ScriptEngine] Failed to load OnCollisionExitScript.");
		}

		// -- Load UpdateDeltaTime --
		rc = s_loadAssemblyAndGetFuncPtr(assemblyPath, managerTypeName, L"UpdateDeltaTime", UNMANAGEDCALLERSONLY_METHOD, nullptr, (void**)&s_updateTimeFunc);
		if (rc != 0 || s_updateTimeFunc == nullptr) {
			AX_CORE_LOG_ERROR("[ScriptEngine] Failed to load UpdateDeltaTime.");
		}

		AX_CORE_LOG_INFO("[ScriptEngine] ScriptManager loaded successfully!");

	}

	void ScriptEngine::shutdown() {
		if (s_context) {
			s_closeFnptr(s_context);
			s_context = nullptr;
		}
		AX_CORE_LOG_INFO("[ScriptEngine] .NET Runtime shutdown");
	}

	bool ScriptEngine::loadHostFxr() {
		// -- Locate hostfxr.dll --
		char_t buffer[MAX_PATH];
		size_t bufferSize = sizeof(buffer) / sizeof(char_t);
		if (get_hostfxr_path(buffer, &bufferSize, nullptr) != 0) {
			AX_CORE_LOG_ERROR("[ScriptEngine] Failed to find hostfxr.dll. Is .NET 10 installed?");
			return false;
		}

		// -- Load the DLL --
		HMODULE lib = LoadLibraryW(buffer);
		if (!lib) {
			AX_CORE_LOG_ERROR("[ScriptEngine] Failed to load hostfxr.dll");
			return false;
		}

		s_initFnptr = (hostfxr_initialize_for_runtime_config_fn)GetProcAddress(lib, "hostfxr_initialize_for_runtime_config");
		s_getDelegateFnptr = (hostfxr_get_runtime_delegate_fn)GetProcAddress(lib, "hostfxr_get_runtime_delegate");
		s_closeFnptr = (hostfxr_close_fn)GetProcAddress(lib, "hostfxr_close");

		return s_initFnptr && s_getDelegateFnptr && s_closeFnptr;
	}

	void* ScriptEngine::createEntityScript(UUID entityID, const char* scriptName) {
		if (s_createEntityScriptFunc) {
			return s_createEntityScriptFunc(entityID.high, entityID.low, scriptName);
		}
		return nullptr;
	}

	void ScriptEngine::destroyEntityScript(void* gcHandle) {
		if (s_destroyEntityScriptFunc && gcHandle != nullptr) {
			s_destroyEntityScriptFunc(gcHandle);
		}
	}

	void ScriptEngine::updateEntityScript(void* gcHandle, float timestep) {
		if (s_updateEntityScriptFunc && gcHandle != nullptr) {
			s_updateEntityScriptFunc(gcHandle, timestep);
		}
	}

	void ScriptEngine::onCollisionEnter(void* gcHandle, Collision& collision) {
		if (s_onCollisionEnterFunc && gcHandle != nullptr) {

			CSharpCollision csharpCol;
			csharpCol.entityIdHigh = collision.other.getComponent<UUIDComponent>().id.high;
			csharpCol.entityIdLow = collision.other.getComponent<UUIDComponent>().id.low;
			csharpCol.contactPoint[0] = collision.contactPoint.x; csharpCol.contactPoint[1] = collision.contactPoint.y; csharpCol.contactPoint[2] = collision.contactPoint.z;
			csharpCol.contactNormal[0] = collision.contactNormal.x; csharpCol.contactNormal[1] = collision.contactNormal.y; csharpCol.contactNormal[2] = collision.contactNormal.z;
			csharpCol.impulse[0] = collision.impulse.x; csharpCol.impulse[1] = collision.impulse.y; csharpCol.impulse[2] = collision.impulse.z;

			s_onCollisionEnterFunc(gcHandle, &csharpCol);
		}
	}

	void ScriptEngine::onCollisionExit(void* gcHandle, Collision& collision) {
		if (s_onCollisionEnterFunc && gcHandle != nullptr) {

			CSharpCollision csharpCol;
			csharpCol.entityIdHigh = collision.other.getComponent<UUIDComponent>().id.high;
			csharpCol.entityIdLow = collision.other.getComponent<UUIDComponent>().id.low;
			csharpCol.contactPoint[0] = collision.contactPoint.x; csharpCol.contactPoint[1] = collision.contactPoint.y; csharpCol.contactPoint[2] = collision.contactPoint.z;
			csharpCol.contactNormal[0] = collision.contactNormal.x; csharpCol.contactNormal[1] = collision.contactNormal.y; csharpCol.contactNormal[2] = collision.contactNormal.z;
			csharpCol.impulse[0] = collision.impulse.x; csharpCol.impulse[1] = collision.impulse.y; csharpCol.impulse[2] = collision.impulse.z;

			s_onCollisionExitFunc(gcHandle, &csharpCol);
		}
	}

	void ScriptEngine::updateTime(float deltaTime) {
		if (s_updateTimeFunc) {
			s_updateTimeFunc(deltaTime);
		}
	}

	void ScriptEngine::setSceneContext(Scene* scene) {
		s_sceneContext = scene;
	}

	Scene* ScriptEngine::getSceneContext() {
		return s_sceneContext;
	}

}
