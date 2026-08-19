#pragma once

#ifdef AX_PLATFORM_WINDOWS
#include <windows.h>
#include <objbase.h>
#endif

#include "AxionEngine/Source/core/Application.h"
#include "AxionEngine/Source/core/Logging.h"
#include "AxionEngine/Source/core/Ref.h"

extern Axion::Application* Axion::createApplication();


namespace Axion {
	inline int EngineMain(int argc, char** argv) {
		Axion::Log::initialize();

		auto app = Axion::createApplication();
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