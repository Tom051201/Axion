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

		inline static std::wstring StringToWString(const std::string& str) {
			if (str.empty()) return {};

			int size_needed = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), (int)str.size(), NULL, 0);
			std::wstring wstr(size_needed, 0);
			MultiByteToWideChar(CP_UTF8, 0, str.c_str(), (int)str.size(), &wstr[0], size_needed);
			return wstr;
		}

	};

}
