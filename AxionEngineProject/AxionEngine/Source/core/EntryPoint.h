#pragma once

#ifdef AX_PLATFORM_WINDOWS
#include <windows.h>
#include <objbase.h>
#endif

#include "AxionEngine/Source/core/Application.h"
#include "AxionEngine/Source/core/Logging.h"
#include "AxionEngine/Source/core/Ref.h"

extern Axion::Application* Axion::createApplication(ApplicationCommandLineArgs args);


namespace Axion {

	inline void SetupWorkingDirectory() {
		#ifdef AX_PLATFORM_WINDOWS
		// -- Check If Already In The Right Directory --
		if (std::filesystem::exists("AxionStudio/Resources")) {
			return;
		}

		char exePath[MAX_PATH];
		GetModuleFileNameA(NULL, exePath, MAX_PATH);
		std::filesystem::path currentPath = exePath;
		bool foundRoot = false;

		while (currentPath.has_parent_path()) {
			std::filesystem::path parent = currentPath.parent_path();
			if (parent == currentPath) break;
			currentPath = parent;

			// -- Check For Developer Folder Structure --
			if (std::filesystem::exists(currentPath / "AxionStudioProject" / "AxionStudio" / "Resources")) {
				std::filesystem::current_path(currentPath / "AxionStudioProject");
				foundRoot = true;
				break;
			}

			// -- Check For Deployed/Packaged Folder Structure --
			if (std::filesystem::exists(currentPath / "AxionStudio" / "Resources")) {
				std::filesystem::current_path(currentPath);
				foundRoot = true;
				break;
			}
		}

		if (!foundRoot) {
			std::string errorMsg = "CRITICAL ERROR: Could not find Engine Root Directory!\n\n"
				"Exe Path was:\n" + std::string(exePath) + "\n\n"
				"Make sure 'AxionStudio/Resources' exists in the project hierarchy.";

			MessageBoxA(NULL, errorMsg.c_str(), "Axion Engine Boot Failure", MB_OK | MB_ICONERROR);
			exit(-1);
		}
	#endif
	}

	inline int EngineMain(int argc, char** argv) {
		SetupWorkingDirectory();

		Axion::Log::initialize();

		Axion::ApplicationCommandLineArgs args;
		args.count = argc;
		args.args = argv;

		auto app = Axion::createApplication(args);
		app->run();
		delete app;

		#ifdef AX_DEBUG
		RefTracker::dump();
		#endif

		return 0;
	}
}

// ---------- DEBUG / RELEASE (with console) ----------
#if defined(AX_PLATFORM_WINDOWS) && (defined(AX_DEBUG) || defined(AX_RELEASE))

int main(int argc, char** argv) {
	HRESULT hr = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
	int result = Axion::EngineMain(argc, argv);
	CoUninitialize();
	return result;
}

#endif

// ---------- DISTRIBUTION (WinMain, no console) ----------
#if defined(AX_PLATFORM_WINDOWS) && defined(AX_DISTRIBUTION)

#include <Windows.h>
int APIENTRY WinMain(_In_ HINSTANCE hInstance, _In_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nCmdShow) {
	HRESULT hr = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
	int result = Axion::EngineMain(__argc, __argv);
	CoUninitialize();
	return result;
}

#endif