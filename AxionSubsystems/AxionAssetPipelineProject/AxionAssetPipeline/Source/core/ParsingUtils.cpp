#include "ParsingUtils.h"

#include "AxionEngine/Vendor/stb_image/stb_image.h"

#include "AxionEngine/Source/core/Logging.h"
#include "AxionEngine/Source/core/Core.h"

namespace Axion::AAP {

	ImageData ParsePNG::loadPNG(const std::string& path) {
		// TODO: add this when creating the binary files for assets
		AX_CORE_ASSERT(false, "Loading PNG data is not supported yet");
		return {};
	}

}
