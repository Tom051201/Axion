#include "axpch.h"
#include "PathResolver.h"

namespace Axion {

	void PathResolver::setAlias(const std::string& alias, const std::filesystem::path& absolutePath) {
		s_aliases[alias] = normalizePath(absolutePath);
	}

	std::filesystem::path PathResolver::resolve(const std::string& virtualPath) {
		std::string pathStr = virtualPath;
		std::replace(pathStr.begin(), pathStr.end(), '\\', '/');
		for (const auto& [alias, absPath] : s_aliases) {
			size_t pos = pathStr.find(alias);
			if (pos != std::string::npos) {
				pathStr.replace(pos, alias.length(), absPath);
				break;
			}
		}

		return std::filesystem::path(pathStr);
	}

	std::string PathResolver::virtualize(const std::filesystem::path& absolutePath) {
		std::string pathStr = normalizePath(std::filesystem::weakly_canonical(absolutePath));
		std::string bestAlias = "";
		std::string bestMatchStr = "";

		for (const auto& [alias, absPath] : s_aliases) {
			if (pathStr.find(absPath) == 0) {
				if (absPath.length() > bestMatchStr.length()) {
					bestMatchStr = absPath;
					bestAlias = alias;
				}
			}
		}

		if (!bestAlias.empty()) {
			pathStr.replace(0, bestMatchStr.length(), bestAlias);
		}

		return pathStr;
	}

	std::string PathResolver::normalizePath(const std::filesystem::path& path) {
		std::string str = path.string();
		std::replace(str.begin(), str.end(), '\\', '/');
		return str;
	}

}
