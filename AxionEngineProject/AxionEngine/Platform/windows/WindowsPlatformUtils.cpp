#include "axpch.h"
#include "AxionEngine/Source/core/PlatformUtils.h"

#include <commdlg.h>
#include <intrin.h>
#include <Windows.h>
#include <winternl.h>
#include <shobjidl.h>
#include <thread>

#include "AxionEngine/Source/core/Application.h"
#include "AxionEngine/Platform/windows/WindowsHelper.h"

namespace Axion {

	namespace FileDialogsInternal {

		std::filesystem::path openFileImpl(const FileDialogs::FilterList& filters, const std::filesystem::path& initialPath) {
			std::filesystem::path result;

			IFileDialog* pfd = nullptr;
			HRESULT hr = CoCreateInstance(CLSID_FileOpenDialog, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pfd));
			if (FAILED(hr)) {
				printf("CoCreateInstance failed: 0x%08lx\n", hr);
				return result;
			}

			// ----- Convert filters to COMDLG_FILTERSPEC -----
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

			// ----- Set initial path -----
			if (!initialPath.empty()) {
				IShellItem* pItem = nullptr;
				if (SUCCEEDED(SHCreateItemFromParsingName(initialPath.c_str(), nullptr, IID_PPV_ARGS(&pItem)))) {
					pfd->SetFolder(pItem);
					pItem->Release();
				}
			}

			// ----- Showing and selecting -----
			hr = pfd->Show(static_cast<HWND>(Application::get().getWindow().getNativeHandle()));
			if (SUCCEEDED(hr)) {
				IShellItem* psi = nullptr;
				hr = pfd->GetResult(&psi);
				if (SUCCEEDED(hr)) {
					PWSTR pszFilePath = nullptr;
					hr = psi->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath);
					if (SUCCEEDED(hr)) {
						result = pszFilePath;
						CoTaskMemFree(pszFilePath);
					}
					psi->Release();
				}
			}

			pfd->Release();
			return result;
		}

		std::filesystem::path saveFileImpl(const FileDialogs::FilterList& filters, const std::filesystem::path& initialPath) {
			std::filesystem::path result;

			IFileDialog* pfd = nullptr;
			HRESULT hr = CoCreateInstance(CLSID_FileSaveDialog, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pfd));
			if (FAILED(hr)) {
				printf("CoCreateInstance failed: 0x%08lx\n", hr);
				return result;
			}

			// ----- Convert filters to COMDLG_FILTERSPEC -----
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

			// ----- Set initial path -----
			if (!initialPath.empty()) {
				IShellItem* pItem = nullptr;
				if (SUCCEEDED(SHCreateItemFromParsingName(initialPath.c_str(), nullptr, IID_PPV_ARGS(&pItem)))) {
					pfd->SetFolder(pItem);
					pItem->Release();
				}
			}

			// ----- Showing and selecting -----
			hr = pfd->Show(static_cast<HWND>(Application::get().getWindow().getNativeHandle()));
			if (SUCCEEDED(hr)) {
				IShellItem* psi = nullptr;
				hr = pfd->GetResult(&psi);
				if (SUCCEEDED(hr)) {
					PWSTR pszFilePath = nullptr;
					hr = psi->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath);
					if (SUCCEEDED(hr)) {
						result = pszFilePath;
						CoTaskMemFree(pszFilePath);
					}
					psi->Release();
				}
			}

			pfd->Release();
			return result;
		}

		std::filesystem::path openFolderImpl(const std::filesystem::path& initialPath) {
			std::filesystem::path result;
			IFileDialog* pfd = nullptr;

			HRESULT hr = CoCreateInstance(CLSID_FileOpenDialog, nullptr, CLSCTX_INPROC_SERVER, IID_IFileDialog, reinterpret_cast<void**>(&pfd));
			if (FAILED(hr)) {
				printf("CoCreateInstance failed: 0x%08lx\n", hr);
				return result;
			}

			DWORD options;
			pfd->GetOptions(&options);
			pfd->SetOptions(options | FOS_PICKFOLDERS);

			if (!initialPath.empty()) {
				IShellItem* pItem = nullptr;
				if (SUCCEEDED(SHCreateItemFromParsingName(initialPath.c_str(), nullptr, IID_PPV_ARGS(&pItem)))) {
					pfd->SetFolder(pItem);
					pItem->Release();
				}
			}

			hr = pfd->Show(static_cast<HWND>(Application::get().getWindow().getNativeHandle()));
			if (SUCCEEDED(hr)) {
				IShellItem* psi;
				hr = pfd->GetResult(&psi);
				if (SUCCEEDED(hr)) {
					PWSTR pszFilePath = nullptr;
					hr = psi->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath);
					if (SUCCEEDED(hr)) {
						result = pszFilePath;
						CoTaskMemFree(pszFilePath);
					}
					psi->Release();
				}
			}

			pfd->Release();
			return result;
		}

		// Helper: ensure STA thread
		template<typename Fn>
		auto runSTA(Fn&& fn) -> decltype(fn()) {
			decltype(fn()) result;
			HRESULT hr = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
			if (SUCCEEDED(hr)) {
				result = fn();
				CoUninitialize();
			}
			else if (hr == RPC_E_CHANGED_MODE) {
				result = fn();
			}
			return result;
		}
		//template<typename Fn>
		//std::string runSTA(Fn&& fn) {
		//	// REMOVE the thread creation entirely
		//	std::string result;
		//	HRESULT hr = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
		//	if (SUCCEEDED(hr)) {
		//		result = fn();           // Run directly on the current thread
		//		CoUninitialize();
		//	}
		//	else if (hr == RPC_E_CHANGED_MODE) {
		//		// COM is already initialized in MTA; you can log a warning
		//		// Or just try running fn() anyway, but some COM features may fail
		//		result = fn();
		//	}
		//	return result;
		//}

	}

	std::filesystem::path FileDialogs::openFile(const FilterList& filters, const std::filesystem::path &initialPath) {
		return FileDialogsInternal::runSTA([&] { return FileDialogsInternal::openFileImpl(filters, initialPath); });
	}

	std::filesystem::path FileDialogs::saveFile(const FilterList& filters, const std::filesystem::path& initialPath) {
		return FileDialogsInternal::runSTA([&] { return FileDialogsInternal::saveFileImpl(filters, initialPath); });
	}

	std::filesystem::path FileDialogs::openFolder(const std::filesystem::path& initialPath) {
		return FileDialogsInternal::runSTA([&] { return FileDialogsInternal::openFolderImpl(initialPath); });
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

		return "Unknown";
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

	void PlatformUtils::showInFileExplorer(const std::filesystem::path& path) {
		std::wstring command = L"explorer.exe /select,\"" + path.wstring() + L"\"";
		_wsystem(command.c_str());
	}

	void PlatformUtils::openFolderInFileExplorer(const std::filesystem::path& path) {
		if (!std::filesystem::exists(path) || !std::filesystem::is_directory(path)) {
			AX_CORE_LOG_WARN("Path is not a folder: {}", path.string());
			return;
		}

		std::wstring command = L"explorer.exe \"" + path.wstring() + L"\"";
		_wsystem(command.c_str());
	}

	std::filesystem::path PlatformUtils::getExecutableDirectory() {
		wchar_t path[MAX_PATH];
		GetModuleFileNameW(nullptr, path, MAX_PATH);
		return std::filesystem::path(path).parent_path();
	}

}
