#include "axpch.h"
#include "ScriptEngine.h"

#include <nethost.h>
#include <coreclr_delegates.h>
#include <hostfxr.h>

namespace Axion {
	
	static hostfxr_initialize_for_runtime_config_fn s_initFnptr = nullptr;
	static hostfxr_get_runtime_delegate_fn s_getDelegateFnptr = nullptr;
	static hostfxr_close_fn s_closeFnptr = nullptr;

	static hostfxr_handle s_context = nullptr;
	static load_assembly_and_get_function_pointer_fn s_loadAssemblyAndGetFuncPtr = nullptr;

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
		const wchar_t* methodName = L"Init";			// Matches the [UnmanagedCallersOnly] EntryPoint name

		typedef void(*initFunc)();
		initFunc initEngineFunc = nullptr;

		rc = s_loadAssemblyAndGetFuncPtr(assemblyPath, typeName, methodName, UNMANAGEDCALLERSONLY_METHOD, nullptr, (void**)&initEngineFunc);
		if (rc != 0 || initEngineFunc == nullptr) {
			AX_CORE_LOG_ERROR("[ScriptEngine] Failed to load C# method. Error code: {0:x}", rc);
			return;
		}

		AX_CORE_LOG_INFO("[ScriptEngine] Executing C# Entry point");
		initEngineFunc();

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

}
