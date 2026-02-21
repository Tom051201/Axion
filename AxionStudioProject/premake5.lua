project "AxionStudio"
	kind "ConsoleApp"
	language "C++"
	cppdialect "C++17"
	staticruntime "on"

	targetdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
	objdir ("%{wks.location}/bin-int/" .. outputdir .. "/%{prj.name}")
	
	buildoptions { "/utf-8" }
	defines { "FMT_UNICODE" }

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
		"%{wks.location}/AxionEngineProject/AxionEngine/Vendor/entt",
		"%{wks.location}/AxionEngineProject/AxionEngine/Vendor/yaml-cpp/include",
		"%{wks.location}/AxionSubsystems/AxionAssetPipelineProject"
	}
	
	links {
		"AxionEngine",
		"AxionAssetPipeline"
	}

	filter "system:windows"
		systemversion "latest"
		buildoptions { "/utf-8" }
		defines {
			"AX_PLATFORM_WINDOWS",
			"YAML_CPP_STATIC_DEFINE"
		}
	
	filter { "system:windows", "configurations:Debug" }
		kind "ConsoleApp"
		defines "AX_DEBUG"
		runtime "Debug"
		symbols "on"
	
	filter { "system:windows", "configurations:Release" }
		kind "ConsoleApp"
		defines "AX_RELEASE"
		runtime "Release"
		optimize "on"
	
	filter { "system:windows", "configurations:Distribution" }
		kind "WindowedApp"
		defines "AX_DISTRIBUTION"
		runtime "Release"
		optimize "on"


	filter "configurations:Debug"
		postbuildcommands {
			"{COPY} " .. PhysXDir .. "/lib/checked/*.dll %{cfg.buildtarget.directory}"
		}

	filter "configurations:Release or Distribution"
		postbuildcommands {
			"{COPY} " .. PhysXDir .. "/lib/release/*.dll %{cfg.buildtarget.directory}"
		}
