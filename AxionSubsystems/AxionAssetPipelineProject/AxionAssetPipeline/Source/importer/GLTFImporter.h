#pragma once

#include <filesystem>

namespace Axion::AAP {

	class GLTFImporter {
	public:

		static void import(const std::filesystem::path& glbPath);

	};

}
