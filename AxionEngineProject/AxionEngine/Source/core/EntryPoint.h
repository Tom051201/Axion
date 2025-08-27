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
	return Axion::EngineMain(argc, argv);
}

#endif

// ---------- DISTRIBUTION (WinMain, no console) ----------
#if defined(AX_PLATFORM_WINDOWS) && defined(AX_DISTRIBUTION)

#include <Windows.h>
int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
	return Axion::EngineMain(__argc, __argv);
}

#endif