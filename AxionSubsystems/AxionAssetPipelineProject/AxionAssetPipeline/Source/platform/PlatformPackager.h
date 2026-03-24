#pragma once

#include <string>

namespace Axion::AAP {

	class PlatformPackager {
	public:

		static bool injectIconIntoExecutable(const std::string& executablePath, const std::string& iconPath);

	};

}
