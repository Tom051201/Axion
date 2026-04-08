#pragma once

#include <string>
#include <filesystem>

namespace Axion::AAP {

	class PlatformPackager {
	public:

		static bool injectIconIntoExecutable(const std::filesystem::path& executablePath, const std::filesystem::path& iconPath);

	};

}
