#pragma once

#include <string>
#include <cstdint>
#include <sstream>

#include "AxionEngine/Source/core/Logging.h"

namespace Axion {

	class Version {
	public:

		uint32_t major = 0;
		uint32_t minor = 0;
		uint32_t patch = 0;

		constexpr Version() = default;
		constexpr Version(uint32_t major, uint32_t minor, uint32_t patch) : major(major), minor(minor), patch(patch) {}

		std::string toString(bool includePrefix = true) const {
			std::string result = std::to_string(major) + "." + std::to_string(minor) + "." + std::to_string(patch);
			return includePrefix ? ("v" + result) : result;
		}

		static Version fromString(const std::string& versionStr) {
			Version v;
			std::string str = versionStr;

			if (!str.empty() && (str[0] == 'v' || str[0] == 'V')) {
				str = str.substr(1);
			}

			std::stringstream ss(str);
			std::string token;

			try {
				if (std::getline(ss, token, '.')) v.major = std::stoul(token);
				if (std::getline(ss, token, '.')) v.minor = std::stoul(token);
				if (std::getline(ss, token, '.')) v.patch = std::stoul(token);
			}
			catch (const std::exception& e) {	
				AX_CORE_LOG_WARN("Failed converting string to version: {}", e.what());
			}

			return v;
		}

		constexpr bool operator==(const Version& other) const {
			return major == other.major && minor == other.minor && patch == other.patch;
		}

		constexpr bool operator!=(const Version& other) const {
			return !(*this == other);
		}

		constexpr bool operator<(const Version& other) const {
			if (major != other.major) return major < other.major;
			if (minor != other.minor) return minor < other.minor;
			return patch < other.patch;
		}

		constexpr bool operator>(const Version& other) const {
			return other < *this;
		}

		constexpr bool operator<=(const Version& other) const {
			return !(other < *this);
		}

		constexpr bool operator>=(const Version& other) const {
			return !(*this < other);
		}

	};

}
