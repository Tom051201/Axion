#pragma once

#include "AxionEngine/Source/core/UUID.h"

namespace Axion {

	struct ScriptAPI {
		uint8_t(*IsKeyDown)(int);
	};

	class ScriptEngine {
	public:

		static void initialize();
		static void shutdown();

		static void* createEntityScript(UUID entityID, const char* scriptName);
		static void destroyEntityScript(void* gcHandle);
		static void updateEntityScript(void* gcHandle, float timestep);

	private:

		static bool loadHostFxr();

	};

}
