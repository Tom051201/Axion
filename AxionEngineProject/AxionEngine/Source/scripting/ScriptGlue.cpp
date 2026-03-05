#include "axpch.h"
#include "ScriptGlue.h"

namespace Axion {

	namespace InternalCalls {

		// -- CPP function pointers --

		extern "C" uint8_t input_isKeyDown(int keyCode) {
			AX_CORE_LOG_TRACE("[CPP ENGINE] C# just asked if key {} is down", keyCode);
			return 1;
		}

	}



	// -- Binding function --
	void ScriptGlue::registerComponents(ScriptAPI& apiStruct) {

		apiStruct.IsKeyDown = InternalCalls::input_isKeyDown;

		AX_CORE_LOG_TRACE("[ScriptGlue] All internal C++ functions registered to C#!");
	}

}
