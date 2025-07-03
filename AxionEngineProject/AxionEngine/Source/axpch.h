#pragma once

// Defaults
#include <algorithm>
#include <cstdint>
#include <filesystem>
#include <functional>
#include <iostream>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>



// Math includes
#include <cmath>
#include <DirectXMath.h>


// Axion
#include "AxionEngine/Source/core/Logging.h"
#include "AxionEngine/Source/core/Timer.h"


// Windows specifics
#ifdef AX_PLATFORM_WINDOWS

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
