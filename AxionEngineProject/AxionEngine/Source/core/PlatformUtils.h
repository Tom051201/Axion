#pragma once

#include <string>

namespace Axion {

	class FileDialogs {
	public:

		// These return empty strings if canceled
		static std::string openFile(const char* filter);
		static std::string saveFile(const char* filter);

	};



	class PlatformInfo {
	public:

		static std::string getOsVersion();

		static std::string getCpuName();
		static uint32_t getCpuCores();
		static uint64_t getRamMB();

	};



	class PlatformUtils {
	public:

		static void showInFileExplorer(const std::string& path);

	};

}
