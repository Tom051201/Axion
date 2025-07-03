#pragma once
#include "axpch.h"

namespace Axion {

	// Define these functions for all
	// platforms. By only compiling the
	// correct cpp file for each platform
	// the correct code will be used
	class PlatformInfo {
	public:

		static std::string getOsVersion();

		static std::string getCpuName();
		static uint32_t getCpuCores();
		static uint64_t getRamMB();

	};

}
