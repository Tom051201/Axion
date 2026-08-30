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
		char exePath[MAX_PATH];
		GetModuleFileNameA(NULL, exePath, MAX_PATH);
		std::filesystem::path currentPath = std::filesystem::path(exePath).parent_path();
		std::string exeName = std::filesystem::path(exePath).stem().string();

		// -- PACKAGED GAME MODE --
		if (std::filesystem::exists(currentPath / "GameConfig.axbin")) {
			std::filesystem::current_path(currentPath);
			return;
		}

		// -- DEV/EDITOR MODE --
		std::filesystem::path searchPath = currentPath;
		while (searchPath.has_parent_path()) {
			if (std::filesystem::exists(searchPath / ".axionroot")) {
				std::filesystem::path targetProjectDir = searchPath / (exeName + "Project");

				if (std::filesystem::exists(targetProjectDir)) {
					std::filesystem::current_path(targetProjectDir);
				}
				else {
					std::filesystem::current_path(searchPath);
				}
				return;
			}

			std::filesystem::path parent = searchPath.parent_path();
			if (parent == searchPath) break;
			searchPath = parent;
		}

		// -- FALLBACK --
		std::filesystem::current_path(currentPath);
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