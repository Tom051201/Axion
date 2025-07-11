workspace "AxionEngineWorkspace"
	architecture "x64"
	configurations {
		"Debug",
		"Release",
		"Distribution"
	}



outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

IncludeDir = {}
IncludeDir["ImGui"] = "AxionEngineProject/AxionEngine/Vendor/imgui"
IncludeDir["GLAD"] = "AxionEngineProject/AxionEngine/Vendor/glad"

include "AxionEngineProject/AxionEngine/Vendor/glad"



project "AxionEngine"
	location "AxionEngineProject"
	kind "StaticLib"
	language "C++"
	cppdialect "C++17"
	staticruntime "on"
	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-int/" .. outputdir .. "/%{prj.name}")
	pchheader "axpch.h"
	pchsource "AxionEngineProject/AxionEngine/Source/axpch.cpp"
	files {
		"AxionEngineProject/AxionEngine/Source/**.h",
		"AxionEngineProject/AxionEngine/Source/**.cpp",
		"AxionEngineProject/AxionEngine/Platform/**.h",
		"AxionEngineProject/AxionEngine/Platform/**.cpp",
		"AxionEngineProject/AxionEngine/Vendor/stb_image/stb_image.h",
		"AxionEngineProject/AxionEngine/Vendor/stb_image/stb_image.cpp"
	}
	includedirs {
		"%{IncludeDir.ImGui}",
		"%{IncludeDir.GLAD}",
		"AxionEngineProject",
		"AxionEngineProject/AxionEngine/Source",
		"AxionEngineProject/AxionEngine/Vendor/spdlog/include",
		"AxionEngineProject/AxionEngine/Vendor/d3d12"
	}
	libdirs {}
	links {
		"ImGui",
		"GLAD"
	}
	filter "system:windows"
		systemversion "latest"
		buildoptions { "/utf-8" }
		defines {
			"AX_PLATFORM_WINDOWS",
			"_CRT_SECURE_NO_WARNINGS",
			"FMT_UNICODE"
		}
	filter "configurations:Debug"
		defines {
			"AX_DEBUG",
			"AX_ENABLE_ASSERTS"
		}
		runtime "Debug"
		symbols "on"
	filter "configurations:Release"
		defines "AX_RELEASE"
		runtime "Release"
		optimize "on"
	filter "configurations:Distribution"
		defines "AX_DISTRIBUTION"
		runtime "Release"
		optimize "on"



project "AxionStudio"
	location "AxionStudioProject"
	kind "ConsoleApp"
	language "C++"
	cppdialect "C++17"
	staticruntime "on"
	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-int/" .. outputdir .. "/%{prj.name}")
	files {
		"AxionStudioProject/AxionStudio/Source/**.h",
		"AxionStudioProject/AxionStudio/Source/**.cpp"
	}
	includedirs {
		"AxionStudioProject",
		"AxionEngineProject",
		"AxionEngineProject/AxionEngine/Vendor/spdlog/include",
		"AxionEngineProject/AxionEngine/Source",
		"AxionEngineProject/AxionEngine/Vendor/imgui",
		"AxionEngineProject/AxionEngine/Vendor/d3d12",
		"AxionEngineProject/AxionEngine/Vendor/entt"
	}
	links {
		"AxionEngine"
	}
	filter "system:windows"
		systemversion "latest"
		buildoptions { "/utf-8" }
		defines {
			"AX_PLATFORM_WINDOWS"
		}
	filter "configurations:Debug"
		defines "AX_DEBUG"
		runtime "Debug"
		symbols "on"
	filter "configurations:Release"
		defines "AX_RELEASE"
		runtime "Release"
		optimize "on"
	filter "configurations:Distribution"
		defines "AX_DISTRIBUTION"
		runtime "Release"
		optimize "on"



project "Sandbox"
	location "SandboxProject"
	kind "ConsoleApp"
	language "C++"
	cppdialect "C++17"
	staticruntime "on"
	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-int/" .. outputdir .. "/%{prj.name}")
	files {
		"SandboxProject/Sandbox/Source/**.h",
		"SandboxProject/Sandbox/Source/**.cpp"
	}
	includedirs {
		"SandboxProject",
		"AxionEngineProject",
		"AxionEngineProject/AxionEngine/Vendor/spdlog/include",
		"AxionEngineProject/AxionEngine/Source",
		"AxionEngineProject/AxionEngine/Vendor/imgui",
		"AxionEngineProject/AxionEngine/Vendor/d3d12",
		"AxionEngineProject/AxionEngine/Vendor/entt"
	}
	links {
		"AxionEngine"
	}
	filter "system:windows"
		systemversion "latest"
		buildoptions { "/utf-8" }
		defines {
			"AX_PLATFORM_WINDOWS"
		}
	filter "configurations:Debug"
		defines "AX_DEBUG"
		runtime "Debug"
		symbols "on"
	filter "configurations:Release"
		defines "AX_RELEASE"
		runtime "Release"
		optimize "on"
	filter "configurations:Distribution"
		defines "AX_DISTRIBUTION"
		runtime "Release"
		optimize "on"



project "ImGui"
	location "AxionEngineProject/AxionEngine/Vendor/imgui"
	kind "StaticLib"
	language "C++"
	cppdialect "C++17"
	staticruntime "On"
	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-int/" .. outputdir .. "/%{prj.name}")
	files {
		"AxionEngineProject/AxionEngine/Vendor/imgui/imconfig.h",
		"AxionEngineProject/AxionEngine/Vendor/imgui/imgui.h",
		"AxionEngineProject/AxionEngine/Vendor/imgui/imgui.cpp",
		"AxionEngineProject/AxionEngine/Vendor/imgui/imgui_draw.cpp",
		"AxionEngineProject/AxionEngine/Vendor/imgui/imgui_internal.h",
		"AxionEngineProject/AxionEngine/Vendor/imgui/imgui_widgets.cpp",
		"AxionEngineProject/AxionEngine/Vendor/imgui/imstb_rectpack.h",
		"AxionEngineProject/AxionEngine/Vendor/imgui/imstb_textedit.h",
		"AxionEngineProject/AxionEngine/Vendor/imgui/imstb_truetype.h",
		"AxionEngineProject/AxionEngine/Vendor/imgui/imgui_demo.cpp",
		"AxionEngineProject/AxionEngine/Vendor/imgui/imgui_tables.cpp",
		"AxionEngineProject/AxionEngine/Vendor/imgui/backends/imgui_impl_win32.h",
		"AxionEngineProject/AxionEngine/Vendor/imgui/backends/imgui_impl_dx12.h",
		"AxionEngineProject/AxionEngine/Vendor/imgui/backends/imgui_impl_opengl3.h",
		"AxionEngineProject/AxionEngine/Vendor/imgui/backends/imgui_impl_opengl3_loader.h",
		"AxionEngineProject/AxionEngine/Vendor/imgui/backends/imgui_impl_win32.cpp",
		"AxionEngineProject/AxionEngine/Vendor/imgui/backends/imgui_impl_dx12.cpp",
		"AxionEngineProject/AxionEngine/Vendor/imgui/backends/imgui_impl_opengl3.cpp"
	}
	includedirs {
		"AxionEngineProject/AxionEngine/Vendor/imgui"
	}
	filter "system:windows"
		systemversion "latest"
	filter "configurations:Debug"
		runtime "Debug"
		symbols "on"
	filter "configurations:Release"
		runtime "Release"
		optimize "on"
