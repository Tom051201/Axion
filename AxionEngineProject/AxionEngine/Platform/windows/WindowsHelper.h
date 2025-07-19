#pragma once
#include "axpch.h"

namespace Axion {

	class WindowsHelper {
	public:

		inline static std::string WStringToString(const std::wstring& wstr) {
			if (wstr.empty()) return {};

			int sizeNeeded = WideCharToMultiByte(CP_UTF8, 0, wstr.data(), (int)wstr.size(), nullptr, 0, nullptr, nullptr);
			std::string result(sizeNeeded, 0);
			WideCharToMultiByte(CP_UTF8, 0, wstr.data(), (int)wstr.size(), &result[0], sizeNeeded, nullptr, nullptr);
			return result;
		}
	};

}
