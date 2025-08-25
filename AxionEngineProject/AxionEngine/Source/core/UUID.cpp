#include "axpch.h"
#include "UUID.h"

namespace Axion {

	UUID::UUID() {
		static thread_local std::mt19937_64 rng{ std::random_device{}() };
		high = rng();
		low = rng();
	}

	std::string UUID::toString() const {
		std::ostringstream oss;
		oss << std::hex << std::setfill('0')
			<< std::setw(16) << high
			<< std::setw(16) << low;

		return oss.str();
	}

	UUID UUID::fromString(const std::string& str) {
		if (str.size() != 32) {
			AX_CORE_ASSERT(false, "UUID string must be 32 hex characters");
			throw std::invalid_argument("UUID string must be 32 hex characters");
		}

		uint64_t hi = std::stoull(str.substr(0, 16), nullptr, 16);
		uint64_t lo = std::stoull(str.substr(16, 16), nullptr, 16);

		return UUID(hi, lo);
	}

}
