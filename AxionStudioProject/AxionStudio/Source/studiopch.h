#pragma once

// -- C++ Standard Library --
#include <algorithm>
#include <array>
#include <cctype>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <functional>
#include <memory>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

// -- Windows Specific --
#ifdef AX_PLATFORM_WINDOWS
	#define WIN32_LEAN_AND_MEAN
	#include <windows.h>
#endif



// -- Third-Party Libraries --
#include <yaml-cpp/yaml.h>
#include <imgui/imgui.h>
