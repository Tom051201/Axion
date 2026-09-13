#pragma once

#include <cstdint>

#include "AxionEngine/Source/core/Version.h"

namespace Axion::Config {

	// ----- ENGINE -----
	inline constexpr Version EngineVersion = Version(0, 2, 0);
	inline constexpr uint32_t MaxSwapchainBuffers = 2;



	// ----- SERIALIZE / BINARY -----
	inline constexpr uint32_t MaxBinaryStringLength = 1024;



	// ----- DIRECTX12 -----
	inline constexpr uint32_t DX12_MaxRtvDescriptors = 16;
	inline constexpr uint32_t DX12_MaxSrvDescriptors = 100000;
	inline constexpr uint32_t DX12_MaxDsvDescriptors = 16;
	inline constexpr uint32_t DX12_SrvHeapReserve = 64;
	inline constexpr uint32_t DX12_MaxTextureSlots = 16;
	inline constexpr bool DX12_EnableDebugSeverityWarning = false;
	inline constexpr bool DX12_EnableDebugGpuBasedValidation = false;



	// ----- DIRECTX11 ------
	inline constexpr bool DX11_EnableDebugSeverityWarning = false;



	// ----- WIN32 -----
	inline constexpr bool Win32_UsingCustomTitleBar = false;


}
