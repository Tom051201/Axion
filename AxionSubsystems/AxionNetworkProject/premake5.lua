project "AxionNetwork"
	kind "StaticLib"
	language "C++"
	cppdialect "C++20"
	staticruntime "on"

	targetdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
	objdir ("%{wks.location}/bin-int/" .. outputdir .. "/%{prj.name}")

	buildoptions { "/utf-8" }

	files {
		"AxionNetwork/Source/**.h",
		"AxionNetwork/Source/**.cpp"
	}

	includedirs {
		".",
		"AxionNetwork",
		"AxionNetwork/Source",
		"%{wks.location}/AxionEngineProject",
		"%{wks.location}/AxionEngineProject/AxionEngine/Source",
		"%{wks.location}/AxionEngineProject/AxionEngine/Vendor",
		"%{wks.location}/AxionEngineProject/AxionEngine/Vendor/spdlog/include",
		"%{wks.location}/AxionEngineProject/AxionEngine/Vendor/entt",
		"AxionNetwork/Vendor/GNS/include"
	}

	libdirs {
		"AxionNetwork/Vendor/GNS/lib"
	}

	links {
		"GameNetworkingSockets"
	}

	filter "system:windows"
		systemversion "latest"
		defines {
			"AX_PLATFORM_WINDOWS",
			"_CRT_SECURE_NO_WARNINGS"
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
