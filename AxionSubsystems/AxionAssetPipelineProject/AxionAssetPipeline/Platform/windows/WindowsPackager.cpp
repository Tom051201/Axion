#include "axpch.h"

#ifdef AX_PLATFORM_WINDOWS

#include "AxionAssetPipeline/Source/platform/PlatformPackager.h"

namespace Axion::AAP {

#pragma pack(push, 2)
	struct ICONDIR {
		WORD idReserved;
		WORD idType;
		WORD idCount;
	};
	struct ICONDIRENTRY {
		BYTE bWidth;
		BYTE bHeight;
		BYTE bColorCount;
		BYTE bReserved;
		WORD wPlanes;
		WORD wBitCount;
		DWORD dwBytesInRes;
		DWORD dwImageOffset;
	};
	struct GRPICONDIRENTRY {
		BYTE bWidth;
		BYTE bHeight;
		BYTE bColorCount;
		BYTE bReserved;
		WORD wPlanes;
		WORD wBitCount;
		DWORD dwBytesInRes;
		WORD nID;
	};
#pragma pack(pop)


	bool PlatformPackager::injectIconIntoExecutable(const std::filesystem::path& exePath, const std::filesystem::path& iconPath) {
		if (iconPath.empty() || !std::filesystem::exists(iconPath)) {
			AX_CORE_LOG_ERROR("Invalid Icon Path: {}", iconPath.string());
			return false;
		}

		std::ifstream icoFile(iconPath, std::ios::binary | std::ios::ate);
		if (!icoFile) {
			AX_CORE_LOG_ERROR("Failed to Open Icon File: {}", iconPath.string());
			return false;
		}

		std::streamsize size = icoFile.tellg();
		icoFile.seekg(0, std::ios::beg);
		std::vector<uint8_t> icoData(size);
		if (!icoFile.read(reinterpret_cast<char*>(icoData.data()), size)) return false;

		ICONDIR* icoDir = reinterpret_cast<ICONDIR*>(icoData.data());
		if (icoDir->idReserved != 0 || icoDir->idType != 1) {
			AX_CORE_LOG_ERROR("Invalid .ico file");
			return false;
		}

		HANDLE hUpdateRes = BeginUpdateResourceW(exePath.c_str(), FALSE);
		if (hUpdateRes == NULL) {
			AX_CORE_LOG_ERROR("Failed to begin resource update on {}. Is it in use?", exePath.string());
			return false;
		}

		std::vector<uint8_t> groupIconData(sizeof(ICONDIR) + (icoDir->idCount * sizeof(GRPICONDIRENTRY)));
		ICONDIR* grpDir = reinterpret_cast<ICONDIR*>(groupIconData.data());
		grpDir->idReserved = 0;
		grpDir->idType = 1;
		grpDir->idCount = icoDir->idCount;

		GRPICONDIRENTRY* grpEntry = reinterpret_cast<GRPICONDIRENTRY*>(groupIconData.data() + sizeof(ICONDIR));
		ICONDIRENTRY* icoEntry = reinterpret_cast<ICONDIRENTRY*>(icoData.data() + sizeof(ICONDIR));

		for (WORD i = 0; i < icoDir->idCount; ++i) {
			grpEntry[i].bWidth = icoEntry[i].bWidth;
			grpEntry[i].bHeight = icoEntry[i].bHeight;
			grpEntry[i].bColorCount = icoEntry[i].bColorCount;
			grpEntry[i].bReserved = icoEntry[i].bReserved;
			grpEntry[i].wPlanes = icoEntry[i].wPlanes;
			grpEntry[i].wBitCount = icoEntry[i].wBitCount;
			grpEntry[i].dwBytesInRes = icoEntry[i].dwBytesInRes;
			grpEntry[i].nID = i + 1;

			UpdateResourceW(
				hUpdateRes,
				(LPCWSTR)RT_ICON,
				MAKEINTRESOURCEW(grpEntry[i].nID),
				MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US),
				icoData.data() + icoEntry[i].dwImageOffset,
				icoEntry[i].dwBytesInRes
			);
		}

		UpdateResourceW(
			hUpdateRes,
			(LPCWSTR)RT_GROUP_ICON,
			MAKEINTRESOURCEW(1),
			MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US),
			groupIconData.data(),
			(DWORD)groupIconData.size()
		);

		return EndUpdateResourceW(hUpdateRes, FALSE);

	}

}

#endif
