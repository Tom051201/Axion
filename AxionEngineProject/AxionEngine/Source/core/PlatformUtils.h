#pragma once

#include <string>
#include <vector>

namespace Axion {

	struct FileDialogFilter {
		std::string name;
		std::string pattern;
	};

	class FileDialogs {
	public:

		using FilterList = std::vector<FileDialogFilter>;

		// These return empty strings if canceled
		static std::string openFile(const FilterList& filters);
		static std::string saveFile(const FilterList& filters);
		static std::string openFolder();

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
