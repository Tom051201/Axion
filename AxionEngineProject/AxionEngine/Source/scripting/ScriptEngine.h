#pragma once

namespace Axion {

	class ScriptEngine {
	public:

		static void initialize();
		static void shutdown();

	private:

		static bool loadHostFxr();

	};

}
