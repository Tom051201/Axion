#pragma once

#include <string>
#include <filesystem>
#include <unordered_map>

namespace Axion {

	class PathResolver {
	public:

		static void setAlias(const std::string& alias, const std::filesystem::path& absolutePath);

		static std::filesystem::path resolve(const std::string& virtualPath);
		static std::string virtualize(const std::filesystem::path& absolutePath);

	private:

		inline static std::unordered_map<std::string, std::string> s_aliases;

		static std::string normalizePath(const std::filesystem::path& path);

	};

}
