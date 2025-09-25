#pragma once

#include "AxionEngine/Source/core/Logging.h"

extern Axion::Application* Axion::createApplication();


namespace Axion {
	inline int EngineMain(int argc, char** argv) {
		Axion::Log::init();

		auto app = Axion::createApplication();
		app->run();
		delete app;

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
int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
	HRESULT hr = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
	int result = Axion::EngineMain(__argc, __argv);
	CoUninitialize();
	return result;
}

#endif