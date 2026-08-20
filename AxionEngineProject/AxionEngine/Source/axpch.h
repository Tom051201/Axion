#pragma once

// -- C++ Standard Library --
#include <algorithm>
#include <array>
#include <atomic>
#include <cstdint>
#include <condition_variable>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iostream>
#include <memory>
#include <mutex>
#include <queue>
#include <random>
#include <sstream>
#include <stdexcept>
#include <string>
#include <thread>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>



// -- Math --
#include <cmath>
#include <DirectXMath.h>



// -- Axion Core --
#include "AxionEngine/Source/core/Logging.h"
#include "AxionEngine/Source/core/Timer.h"
#include "AxionEngine/Source/core/Ref.h"



// -- Windows Specific --
#ifdef AX_PLATFORM_WINDOWS
	#define WIN32_LEAN_AND_MEAN
	#include <windows.h>

	#include <Windowsx.h>
	#include <wrl.h>

	#include <d3d12.h>
	#include <d3dcompiler.h>
	#include <d3dx12/d3dx12.h>
	#include <dxgi1_6.h>
	#include <dxgidebug.h>

	#pragma comment(lib, "d3d12.lib")
	#pragma comment(lib, "d3dcompiler.lib")
	#pragma comment(lib, "dxcompiler.lib")
	#pragma comment(lib, "dxgi.lib")
	#pragma comment(lib, "dxguid.lib")
#endif
