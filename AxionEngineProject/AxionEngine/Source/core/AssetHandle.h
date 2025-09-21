#pragma once
#include "axpch.h"

#include "AxionEngine/Source/core/UUID.h"

namespace Axion {

	template<typename T>
	struct AssetHandle {
		UUID uuid;

		AssetHandle() : uuid() {}
		AssetHandle(const UUID& uuid) : uuid(uuid) {}

		bool operator==(const AssetHandle& other) const { return uuid == other.uuid; }
		bool operator!=(const AssetHandle& other) const { return !(*this == other); };
	};

}

namespace std {
	template<typename T>
	struct hash<Axion::AssetHandle<T>> {
		size_t operator()(const Axion::AssetHandle<T>& handle) const noexcept {
			return std::hash<Axion::UUID>()(handle.uuid);
		}
	};
}

namespace fmt {

	template<typename T>
	struct formatter<Axion::AssetHandle<T>> {
		constexpr auto parse(format_parse_context& ctx) { return ctx.begin(); }

		template<typename FormatContext>
		auto format(const Axion::AssetHandle<T>& handle, FormatContext& ctx) const {
			return format_to(ctx.out(), "AssetHandle<{}>( {} )", typeid(T).name(), handle.uuid);
		}
	};
}
