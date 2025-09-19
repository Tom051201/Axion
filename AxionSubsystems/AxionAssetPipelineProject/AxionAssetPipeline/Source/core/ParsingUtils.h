#pragma once

#include <vector>
#include <cstdint>
#include <string>

namespace Axion::AAP {

	struct ImageData {
		int width, height;
		int channels;
		std::vector<uint8_t> pixels;
	};

	class ParsePNG {
	public:

		static ImageData loadPNG(const std::string& path);

	};

}
