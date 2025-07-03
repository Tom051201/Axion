#include "axpch.h"
#include "AxionEngine/Source/core/PlatformInfo.h"

#include <intrin.h>
#include <Windows.h>
#include <winternl.h>

namespace Axion {

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
					return std::string(wstr.begin(), wstr.end());
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
