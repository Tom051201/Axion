#include "axpch.h"
#include "AxionEngine/Source/core/PlatformUtils.h"

#include <commdlg.h>
#include <intrin.h>
#include <Windows.h>
#include <winternl.h>
#include <shobjidl.h> 

#include "AxionEngine/Source/core/Application.h"
#include "AxionEngine/Platform/windows/WindowsHelper.h"

namespace Axion {

	std::string FileDialogs::openFile(const FilterList& filters) {
		std::string result;
		HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
		if (SUCCEEDED(hr)) {
			IFileDialog* pfd = nullptr;
			hr = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_ALL, IID_PPV_ARGS(&pfd));

			if (SUCCEEDED(hr)) {
				// Convert filters to COMDLG_FILTERSPEC
				std::vector<std::wstring> ownedNames;
				std::vector<std::wstring> ownedPatterns;
				std::vector<COMDLG_FILTERSPEC> specs;

				for (auto& f : filters) {
					ownedNames.emplace_back(f.name.begin(), f.name.end());
					ownedPatterns.emplace_back(f.pattern.begin(), f.pattern.end());
					specs.push_back({ ownedNames.back().c_str(), ownedPatterns.back().c_str() });
				}

				if (!specs.empty()) {
					pfd->SetFileTypes(static_cast<UINT>(specs.size()), specs.data());
					pfd->SetFileTypeIndex(1); // 1-based index, default to first filter
				}

				hr = pfd->Show(static_cast<HWND>(Application::get().getWindow().getNativeHandle()));
				if (SUCCEEDED(hr)) {
					IShellItem* psi = nullptr;
					hr = pfd->GetResult(&psi);
					if (SUCCEEDED(hr)) {
						PWSTR pszFilePath = nullptr;
						hr = psi->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath);
						if (SUCCEEDED(hr)) {
							char path[MAX_PATH];
							WideCharToMultiByte(CP_ACP, 0, pszFilePath, -1, path, MAX_PATH, NULL, NULL);
							result = path;
							CoTaskMemFree(pszFilePath);
						}
						psi->Release();
					}
				}
				pfd->Release();
			}
			CoUninitialize();
		}

		return result;
	}

	std::string FileDialogs::saveFile(const FilterList& filters) {
		std::string result;

		HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
		if (SUCCEEDED(hr)) {
			IFileDialog* pfd = nullptr;
			hr = CoCreateInstance(CLSID_FileSaveDialog, NULL, CLSCTX_ALL, IID_PPV_ARGS(&pfd));

			if (SUCCEEDED(hr)) {
				// Convert filters to COMDLG_FILTERSPEC
				std::vector<std::wstring> ownedNames;
				std::vector<std::wstring> ownedPatterns;
				std::vector<COMDLG_FILTERSPEC> specs;

				for (auto& f : filters) {
					ownedNames.emplace_back(f.name.begin(), f.name.end());
					ownedPatterns.emplace_back(f.pattern.begin(), f.pattern.end());
					specs.push_back({ ownedNames.back().c_str(), ownedPatterns.back().c_str() });
				}

				if (!specs.empty()) {
					pfd->SetFileTypes(static_cast<UINT>(specs.size()), specs.data());
					pfd->SetFileTypeIndex(1);
				}

				hr = pfd->Show(static_cast<HWND>(Application::get().getWindow().getNativeHandle()));
				if (SUCCEEDED(hr)) {
					IShellItem* psi = nullptr;
					hr = pfd->GetResult(&psi);
					if (SUCCEEDED(hr)) {
						PWSTR pszFilePath = nullptr;
						hr = psi->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath);
						if (SUCCEEDED(hr)) {
							char path[MAX_PATH];
							WideCharToMultiByte(CP_ACP, 0, pszFilePath, -1, path, MAX_PATH, NULL, NULL);
							result = path;
							CoTaskMemFree(pszFilePath);
						}
						psi->Release();
					}
				}
				pfd->Release();
			}
			CoUninitialize();
		}

		return result; // empty if canceled
	}

	std::string FileDialogs::openFolder() {
		IFileDialog* pfd = nullptr;
		std::string result;

		HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
		if (SUCCEEDED(hr)) {
			hr = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_ALL, IID_IFileDialog, reinterpret_cast<void**>(&pfd));

			if (SUCCEEDED(hr)) {
				DWORD options;
				pfd->GetOptions(&options);
				pfd->SetOptions(options | FOS_PICKFOLDERS); // allow folder selection

				hr = pfd->Show(static_cast<HWND>(Application::get().getWindow().getNativeHandle()));
				if (SUCCEEDED(hr)) {
					IShellItem* psi;
					hr = pfd->GetResult(&psi);
					if (SUCCEEDED(hr)) {
						PWSTR pszFilePath = nullptr;
						hr = psi->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath);
						if (SUCCEEDED(hr)) {
							char path[MAX_PATH];
							WideCharToMultiByte(CP_ACP, 0, pszFilePath, -1, path, MAX_PATH, NULL, NULL);
							result = path;
							CoTaskMemFree(pszFilePath);
						}
						psi->Release();
					}
				}
				pfd->Release();
			}
			CoUninitialize();
		}

		return result;
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

	void PlatformUtils::showInFileExplorer(const std::string& path) {
		std::wstring wpath(path.begin(), path.end());
		std::wstring command = L"explorer.exe /select,\"" + wpath + L"\"";
		_wsystem(command.c_str());
	}

}
