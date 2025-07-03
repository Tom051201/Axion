#pragma once

#include "AxionEngine/Source/core/Logging.h"

#ifdef AX_PLATFORM_WINDOWS

extern Axion::Application* Axion::createApplication();

int main(int argc, char** argv) {
	
	Axion::Log::init();

	auto app = Axion::createApplication();
	app->run();
	delete app;

}

#endif
