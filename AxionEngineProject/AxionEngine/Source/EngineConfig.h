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
	inline constexpr uint32_t D12MaxRtvDescriptors = 16;
	inline constexpr uint32_t D12MaxSrvDescriptors = 1024;
	inline constexpr uint32_t D12MaxDsvDescriptors = 16;
	inline constexpr uint32_t D12SrvHeapReserve = 64;
	inline constexpr uint32_t D12MaxTextureSlots = 16;
	inline constexpr bool D12EnableDebugSeverityWarning = false;
	inline constexpr bool D12EnableDebugGpuBasedValidation = false;



	// ----- WIN32 -----
	inline constexpr bool WinUsingCustomTitleBar = false;



	// ----- IMGUI -----
	inline constexpr uint32_t ImguiFramesInFlight = 3;

}
