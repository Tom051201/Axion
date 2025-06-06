#pragma once
#include "Logging.h"

#ifdef AX_PLATFORM_WINDOWS

extern Axion::Application* Axion::createApplication();

int main(int argc, char** argv) {
	
	Axion::Log::init();
	AX_CORE_LOG_WARN("Initialized Log");
	AX_LOG_INFO("Initialized Log");

	auto app = Axion::createApplication();
	app->run();
	delete app;

}

#endif
