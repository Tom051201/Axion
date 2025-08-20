project "AxionStudio"
	kind "ConsoleApp"
	language "C++"
	cppdialect "C++17"
	staticruntime "on"

	targetdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
	objdir ("%{wks.location}/bin-int/" .. outputdir .. "/%{prj.name}")
	
	files {
		"AxionStudio/Source/**.h",
		"AxionStudio/Source/**.cpp"
	}
	
	includedirs {
		".",
		"%{wks.location}/AxionEngineProject",
		"%{wks.location}/AxionEngineProject/AxionEngine/Vendor/spdlog/include",
		"%{wks.location}/AxionEngineProject/AxionEngine/Source",
		"%{wks.location}/AxionEngineProject/AxionEngine/Vendor/imgui",
		"%{wks.location}/AxionEngineProject/AxionEngine/Vendor/d3d12",
		"%{wks.location}/AxionEngineProject/AxionEngine/Vendor/entt"
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
