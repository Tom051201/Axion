workspace "AxionEngineWorkspace"
	architecture "x64"
	configurations {
		"Debug",
		"Release",
		"Distribution"
	}



outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

IncludeDir = {}
IncludeDir["ImGui"] = "AxionEngine/vendor/imgui"



project "AxionEngine"
	location "AxionEngine"
	kind "StaticLib"
	language "C++"
	cppdialect "C++17"
	staticruntime "on"
	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-int/" .. outputdir .. "/%{prj.name}")
	pchheader "axpch.h"
	pchsource "AxionEngine/source/axpch.cpp"
	files {
		"%{prj.name}/source/**.h",
		"%{prj.name}/source/**.cpp",
		"%{prj.name}/vendor/stb_image/stb_image.h",
		"%{prj.name}/vendor/stb_image/stb_image.cpp"
	}
	includedirs {
		"%{prj.name}/vendor/spdlog/include",
		"%{IncludeDir.ImGui}",
		"%{prj.name}/vendor",
		"%{prj.name}/source",
		"%{prj.name}/vendor/d3d12"
	}
	libdirs {}
	links {
		"ImGui"
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
	location "AxionStudio"
	kind "ConsoleApp"
	language "C++"
	cppdialect "C++17"
	staticruntime "on"
	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-int/" .. outputdir .. "/%{prj.name}")
	files {
		"%{prj.name}/source/**.h",
		"%{prj.name}/source/**.cpp"
	}
	includedirs {
		"AxionEngine/vendor/spdlog/include",
		"AxionEngine/source",
		"AxionEngine/vendor/imgui",
		"AxionEngine/vendor/d3d12"
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
	location "Sandbox"
	kind "ConsoleApp"
	language "C++"
	cppdialect "C++17"
	staticruntime "on"
	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-int/" .. outputdir .. "/%{prj.name}")
	files {
		"%{prj.name}/source/**.h",
		"%{prj.name}/source/**.cpp"
	}
	includedirs {
		"AxionEngine/vendor/spdlog/include",
		"AxionEngine/source",
		"AxionEngine/vendor/imgui",
		"AxionEngine/vendor/d3d12"
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
	location "AxionEngine/vendor/imgui"
	kind "StaticLib"
	language "C++"
	cppdialect "C++17"
	staticruntime "On"
	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-int/" .. outputdir .. "/%{prj.name}")
	files {
		"AxionEngine/vendor/imgui/imconfig.h",
		"AxionEngine/vendor/imgui/imgui.h",
		"AxionEngine/vendor/imgui/imgui.cpp",
		"AxionEngine/vendor/imgui/imgui_draw.cpp",
		"AxionEngine/vendor/imgui/imgui_internal.h",
		"AxionEngine/vendor/imgui/imgui_widgets.cpp",
		"AxionEngine/vendor/imgui/imstb_rectpack.h",
		"AxionEngine/vendor/imgui/imstb_textedit.h",
		"AxionEngine/vendor/imgui/imstb_truetype.h",
		"AxionEngine/vendor/imgui/imgui_demo.cpp",
		"AxionEngine/vendor/imgui/imgui_tables.cpp",
		"AxionEngine/vendor/imgui/backends/imgui_impl_win32.h",
		"AxionEngine/vendor/imgui/backends/imgui_impl_dx12.h",
		"AxionEngine/vendor/imgui/backends/imgui_impl_win32.cpp",
		"AxionEngine/vendor/imgui/backends/imgui_impl_dx12.cpp"
	}
	includedirs {
		"AxionEngine/vendor/imgui"
	}
	filter "system:windows"
		systemversion "latest"
	filter "configurations:Debug"
		runtime "Debug"
		symbols "on"
	filter "configurations:Release"
		runtime "Release"
		optimize "on"
