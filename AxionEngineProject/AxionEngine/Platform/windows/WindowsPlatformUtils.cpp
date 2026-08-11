#include "axpch.h"
#include "AxionEngine/Source/core/PlatformUtils.h"

#include <commdlg.h>
#include <intrin.h>
#include <Windows.h>
#include <winternl.h>
#include <shobjidl.h>
#include <processthreadsapi.h>
#include <shellapi.h>
#include <Shlwapi.h>
#pragma comment(lib, "Shlwapi.lib")

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
				std::filesystem::path absPath = std::filesystem::absolute(initialPath);
				absPath.make_preferred();

				IShellItem* pItem = nullptr;
				if (SUCCEEDED(SHCreateItemFromParsingName(absPath.c_str(), nullptr, IID_PPV_ARGS(&pItem)))) {
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
				std::filesystem::path absPath = std::filesystem::absolute(initialPath);
				absPath.make_preferred();

				IShellItem* pItem = nullptr;
				if (SUCCEEDED(SHCreateItemFromParsingName(absPath.c_str(), nullptr, IID_PPV_ARGS(&pItem)))) {
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
				std::filesystem::path absPath = std::filesystem::absolute(initialPath);
				absPath.make_preferred();

				IShellItem* pItem = nullptr;
				if (SUCCEEDED(SHCreateItemFromParsingName(absPath.c_str(), nullptr, IID_PPV_ARGS(&pItem)))) {
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

	void PlatformUtils::openExternally(const std::filesystem::path& path) {
		if (!std::filesystem::exists(path)) return;

		ShellExecuteW(nullptr, L"open", path.wstring().c_str(), nullptr, nullptr, SW_SHOW);
	}

	std::string PlatformUtils::getDefaultProgramName(const std::filesystem::path& path) {
		if (!path.has_extension()) return "";
		std::string extString = path.extension().string();

		// -- Time Based Cache --
		struct CacheEntry {
			std::string name;
			std::chrono::steady_clock::time_point timestamp;
		};
		static std::unordered_map<std::string, CacheEntry> s_ProgramCache;

		auto now = std::chrono::steady_clock::now();
		if (s_ProgramCache.find(extString) != s_ProgramCache.end()) {
			if (std::chrono::duration_cast<std::chrono::seconds>(now - s_ProgramCache[extString].timestamp).count() < 5) {
				return s_ProgramCache[extString].name;
			}
		}

		std::wstring ext = path.extension().wstring();
		DWORD bufferSize = 0;

		// -- Create Text Buffer --
		HRESULT hr = AssocQueryStringW(ASSOCF_INIT_IGNOREUNKNOWN, ASSOCSTR_FRIENDLYAPPNAME, ext.c_str(), NULL, NULL, &bufferSize);
		if (FAILED(hr) && hr != HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER)) {
			s_ProgramCache[extString] = { "", now };
			return "";
		}

		// -- Get Program Name --
		std::vector<wchar_t> buffer(bufferSize);
		hr = AssocQueryStringW(ASSOCF_INIT_IGNOREUNKNOWN, ASSOCSTR_FRIENDLYAPPNAME, ext.c_str(), NULL, buffer.data(), &bufferSize);
		if (SUCCEEDED(hr)) {
			// -- Convert UTF-16 To UTF-8 --
			int size_needed = WideCharToMultiByte(CP_UTF8, 0, buffer.data(), -1, NULL, 0, NULL, NULL);
			if (size_needed > 0) {
				std::string result(size_needed - 1, 0);
				WideCharToMultiByte(CP_UTF8, 0, buffer.data(), -1, &result[0], size_needed - 1, NULL, NULL);

				s_ProgramCache[extString] = { result, now };
				return result;
			}
		}

		s_ProgramCache[extString] = { "", now };
		return "";
	}

	std::filesystem::path PlatformUtils::getExecutableDirectory() {
		wchar_t path[MAX_PATH];
		GetModuleFileNameW(nullptr, path, MAX_PATH);
		return std::filesystem::path(path).parent_path();
	}

	void PlatformUtils::setCurrentThreadName(const std::string& name) {
		int size_needed = MultiByteToWideChar(CP_UTF8, 0, &name[0], (int)name.size(), NULL, 0);
		std::wstring wname(size_needed, 0);
		MultiByteToWideChar(CP_UTF8, 0, &name[0], (int)name.size(), &wname[0], size_needed);

		SetThreadDescription(GetCurrentThread(), wname.c_str());
	}

	void PlatformUtils::setThreadPriority(ThreadPriority priority) {
		int winPriority = THREAD_PRIORITY_NORMAL;

		switch (priority) {
			case ThreadPriority::Idle:			winPriority = THREAD_PRIORITY_IDLE; break;
			case ThreadPriority::Lowest:		winPriority = THREAD_PRIORITY_LOWEST; break;
			case ThreadPriority::BelowNormal:	winPriority = THREAD_PRIORITY_BELOW_NORMAL; break;
			case ThreadPriority::Normal:		winPriority = THREAD_PRIORITY_NORMAL; break;
			case ThreadPriority::AboveNormal:	winPriority = THREAD_PRIORITY_ABOVE_NORMAL; break;
			case ThreadPriority::Highest:		winPriority = THREAD_PRIORITY_HIGHEST; break;
			case ThreadPriority::TimeCritical:	winPriority = THREAD_PRIORITY_TIME_CRITICAL; break;
		}

		SetThreadPriority(GetCurrentThread(), winPriority);
	}

}
