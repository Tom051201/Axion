#include "axpch.h"
#include "AxionEngine/Source/core/PlatformUtils.h"

#include <commdlg.h>
#include <intrin.h>
#include <Windows.h>
#include <winternl.h>

#include "AxionEngine/Source/core/Application.h"
#include "AxionEngine/Platform/windows/WindowsHelper.h"

namespace Axion {

	std::string FileDialogs::openFile(const char* filter) {

		OPENFILENAMEA ofn;
		CHAR szFile[260] = { 0 };
		ZeroMemory(&ofn, sizeof(OPENFILENAME));
		ofn.lStructSize = sizeof(OPENFILENAME);
		ofn.hwndOwner = static_cast<HWND>(Application::get().getWindow().getNativeHandle());
		ofn.lpstrFile = szFile;
		ofn.nMaxFile = sizeof(szFile);
		ofn.lpstrFilter = filter;
		ofn.nFilterIndex = 1;
		ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;
		if (GetOpenFileNameA(&ofn) == TRUE) {
			return ofn.lpstrFile;
		}
		return std::string();
	}

	std::string FileDialogs::saveFile(const char* filter) {
		OPENFILENAMEA ofn;
		CHAR szFile[260] = { 0 };
		ZeroMemory(&ofn, sizeof(OPENFILENAME));
		ofn.lStructSize = sizeof(OPENFILENAME);
		ofn.hwndOwner = static_cast<HWND>(Application::get().getWindow().getNativeHandle());
		ofn.lpstrFile = szFile;
		ofn.nMaxFile = sizeof(szFile);
		ofn.lpstrFilter = filter;
		ofn.nFilterIndex = 1;
		ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;
		if (GetSaveFileNameA(&ofn) == TRUE) {
			return ofn.lpstrFile;
		}
		return std::string();
	}

	std::string PlatformInfo::getOsVersion() {
		typedef LONG(WINAPI* RtlGetVersionPtr)(PRTL_OSVERSIONINFOW);
		HMODULE hMod = ::GetModuleHandleW(L"ntdll.dll");
		if (hMod) {
			RtlGetVersionPtr fxPtr = (RtlGetVersionPtr)::GetProcAddress(hMod, "RtlGetVersion");
			if (fxPtr != nullptr) {
				RTL_OSVERSIONINFOW rovi = { 0 };
				rovi.dwOSVersionInfoSize = sizeof(rovi);
				if (fxPtr(&rovi) == 0) {
					wchar_t buf[128];
					swprintf_s(buf, 128, L"Windows %d.%d (Build %d)", rovi.dwMajorVersion, rovi.dwMinorVersion, rovi.dwBuildNumber);
					std::wstring wstr(buf);
					return WindowsHelper::WStringToString(wstr);
				}
			}
		}

		return "Unknown";
	}

	std::string PlatformInfo::getCpuName() {
		char cpuBrand[0x40] = {};
		int cpuInfo[4] = {};
		__cpuid(cpuInfo, 0x80000000);
		unsigned int maxExtended = cpuInfo[0];

		if (maxExtended >= 0x80000004) {
			__cpuid((int*)cpuBrand, 0x80000002);
			__cpuid((int*)(cpuBrand + 16), 0x80000003);
			__cpuid((int*)(cpuBrand + 32), 0x80000004);
			return cpuBrand;
		}

		return "Unkown";
	}

	uint32_t PlatformInfo::getCpuCores() {
		SYSTEM_INFO sysInfo;
		GetSystemInfo(&sysInfo);
		return sysInfo.dwNumberOfProcessors;
	}

	uint64_t PlatformInfo::getRamMB() {
		MEMORYSTATUSEX memInfo = {};
		memInfo.dwLength = sizeof(memInfo);
		if (GlobalMemoryStatusEx(&memInfo)) {
			return static_cast<uint64_t>(memInfo.ullTotalPhys / (1024 * 1024));
		}

		return 0;
	}

}
