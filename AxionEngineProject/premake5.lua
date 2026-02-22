project "AxionEngine"
	kind "StaticLib"
	language "C++"
	cppdialect "C++17"
	staticruntime "on"

	targetdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
	objdir ("%{wks.location}/bin-int/" .. outputdir .. "/%{prj.name}")

	buildoptions { "/utf-8" }
	defines { "FMT_UNICODE" }

	pchheader "axpch.h"
	pchsource "AxionEngine/Source/axpch.cpp"
	
	files {
		"AxionEngine/Source/**.h",
		"AxionEngine/Source/**.cpp",
		"AxionEngine/Platform/**.h",
		"AxionEngine/Platform/**.cpp",
		"AxionEngine/Vendor/stb_image/stb_image.h",
		"AxionEngine/Vendor/stb_image/stb_image.cpp",
		"AxionEngine/Vendor/tinyobjloader/tiny_obj_loader.h",
		"AxionEngine/Vendor/ImGuizmo/ImGuizmo.h",
		"AxionEngine/Vendor/ImGuizmo/ImGuizmo.cpp"
	}
	
	includedirs {
		".",
		"%{IncludeDir.ImGui}",
		"%{IncludeDir.GLAD}",
		"%{IncludeDir.yaml_cpp}",
		"%{IncludeDir.PhysX}",
		"AxionEngine/Source",
		"AxionEngine/Vendor/spdlog/include",
		"AxionEngine/Vendor/d3d12",
		"AxionEngine/Vendor/yaml-cpp/include",
		"AxionEngine/Vendor/ImGuizmo"
	}
	
	libdirs {}
	
	links {
		"ImGui",
		"GLAD",
		"yaml-cpp",
		"PhysXFoundation_64",
		"PhysX_64",
		"PhysXCooking_64",
		"PhysXCommon_64",
		"PhysXExtensions_static_64",
		"PhysXPvdSDK_static_64"
	}

	filter "files:AxionEngine/Vendor/ImGuizmo/**.cpp"
		flags { "NoPCH" }
	
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
			"_DEBUG",
			"AX_DEBUG",
			"AX_ENABLE_ASSERTS"
		}
		runtime "Debug"
		symbols "on"
		libdirs { "%{wks.location}/AxionEngineProject/AxionEngine/Vendor/physx/lib/debug" }
		linkoptions { "/IGNORE:4006" }
	
	filter "configurations:Release"
		defines {
			"NDEBUG",
			"AX_RELEASE"
		}
		runtime "Release"
		optimize "on"
		libdirs { "%{wks.location}/AxionEngineProject/AxionEngine/Vendor/physx/lib/release" }
	
	filter "configurations:Distribution"
		defines {
			"NDEBUG",
			"AX_DISTRIBUTION"
		}
		runtime "Release"
		optimize "on"
		libdirs { "%{wks.location}/AxionEngineProject/AxionEngine/Vendor/physx/lib/release" }
