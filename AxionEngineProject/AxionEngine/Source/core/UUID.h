#pragma once
#include "axpch.h"

namespace Axion {

	class UUID {
	public:

		uint64_t high = 0;
		uint64_t low = 0;

		UUID() = default;
		UUID(uint64_t high, uint64_t low) : high(high), low(low) {}

		static UUID generate();
		bool isValid() const { return high != 0 && low != 0; } // TODO: replace && with ||

		std::string toString() const;
		static UUID fromString(const std::string& str);

		bool operator==(const UUID& other) const { return high == other.high && low == other.low; }
		bool operator!=(const UUID& other) const { return !(*this == other); }
		bool operator<(const UUID& other) const { return (high < other.high) || (high == other.high && low < other.low); }

	};

}

namespace std {

	template<>
	struct hash<Axion::UUID> {
		std::size_t operator()(const Axion::UUID& id) const noexcept {
			return std::hash<uint64_t>()(id.high ^ id.low);
		}
	};

}

namespace fmt {

	template<>
	struct formatter<Axion::UUID> {
		constexpr auto parse(format_parse_context& ctx) { return ctx.begin(); }

		template<typename FormatContext>
		auto format(const Axion::UUID& id, FormatContext& ctx) const {
			return format_to(ctx.out(), id.toString());
		}

	};

}
