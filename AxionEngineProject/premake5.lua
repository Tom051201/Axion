project "AxionEngine"
	kind "StaticLib"
	language "C++"
	cppdialect "C++17"
	staticruntime "on"

	targetdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
	objdir ("%{wks.location}/bin-int/" .. outputdir .. "/%{prj.name}")

	pchheader "axpch.h"
	pchsource "AxionEngine/Source/axpch.cpp"
	
	files {
		"AxionEngine/Source/**.h",
		"AxionEngine/Source/**.cpp",
		"AxionEngine/Platform/**.h",
		"AxionEngine/Platform/**.cpp",
		"AxionEngine/Vendor/stb_image/stb_image.h",
		"AxionEngine/Vendor/stb_image/stb_image.cpp"
	}
	
	includedirs {
		".",
		"%{IncludeDir.ImGui}",
		"%{IncludeDir.GLAD}",
		"%{IncludeDir.yaml_cpp}",
		"AxionEngine/Source",
		"AxionEngine/Vendor/spdlog/include",
		"AxionEngine/Vendor/d3d12",
		"AxionEngine/Vendor/yaml-cpp/include"
	}
	
	libdirs {}
	
	links {
		"ImGui",
		"GLAD",
		"yaml-cpp"
	}
	
	filter "system:windows"
		systemversion "latest"
		buildoptions { "/utf-8" }
		defines {
			"AX_PLATFORM_WINDOWS",
			"_CRT_SECURE_NO_WARNINGS",
			"FMT_UNICODE",
			"YAML_CPP_STATIC_DEFINE"
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
