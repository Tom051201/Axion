#pragma once

#include <string>
#include <vector>
#include <filesystem>

namespace Axion {

	struct FileDialogFilter {
		std::string name;
		std::string pattern;
	};

	class FileDialogs {
	public:

		using FilterList = std::vector<FileDialogFilter>;

		// These return empty strings if canceled
		static std::filesystem::path openFile(const FilterList& filters, const std::filesystem::path &initialPath = "");
		static std::filesystem::path saveFile(const FilterList& filters, const std::filesystem::path& initialPath = "");
		static std::filesystem::path openFolder(const std::filesystem::path& initialPath = "");

	};



	class PlatformInfo {
	public:

		static std::string getOsVersion();

		static std::string getCpuName();
		static uint32_t getCpuCores();
		static uint64_t getRamMB();

	};



	enum class ThreadPriority {
		Idle,
		Lowest,
		BelowNormal,
		Normal,
		AboveNormal,
		Highest,
		TimeCritical
	};

	class PlatformUtils {
	public:

		static void showInFileExplorer(const std::filesystem::path& path);
		static void openFolderInFileExplorer(const std::filesystem::path& path);

		static void openExternally(const std::filesystem::path& path);
		static std::string getDefaultProgramName(const std::filesystem::path& path);

		static std::filesystem::path getExecutableDirectory();

		static void setCurrentThreadName(const std::string& name);
		static void setThreadPriority(ThreadPriority priority);

		static void registerProjectFileExtension();

	};

}
