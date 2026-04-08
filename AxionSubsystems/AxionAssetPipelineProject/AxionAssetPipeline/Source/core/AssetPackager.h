#pragma once

#include <string>
#include <filesystem>

namespace Axion::AAP {

	class AssetPackager {
	public:

		static void packageProject(const std::filesystem::path& outputDirectory);

	private:

		static std::filesystem::path getRuntimePath(const std::filesystem::path& inPath);

	};

}
