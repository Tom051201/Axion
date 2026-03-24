project "AxionRuntime"
	kind "ConsoleApp"
	language "C++"
	cppdialect "C++17"
	staticruntime "on"

	targetdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
	objdir ("%{wks.location}/bin-int/" .. outputdir .. "/%{prj.name}")

	debugdir ("%{wks.location}/RuntimeTestProject")

	buildoptions { "/utf-8" }
	defines {
		"FMT_UNICODE",
		"AX_RUNTIME"
	}

	files {
		"AxionRuntime/Source/**.h",
		"AxionRuntime/Source/**.cpp"
	}
	
	includedirs {
		".",
		"%{wks.location}/AxionEngineProject",
		"%{wks.location}/AxionEngineProject/AxionEngine/Vendor/spdlog/include",
		"%{wks.location}/AxionEngineProject/AxionEngine/Source",
		"%{wks.location}/AxionEngineProject/AxionEngine/Vendor/imgui",
		"%{wks.location}/AxionEngineProject/AxionEngine/Vendor/d3d12",
		"%{wks.location}/AxionEngineProject/AxionEngine/Vendor/entt",
		"%{wks.location}/AxionEngineProject/AxionEngine/Vendor/yaml-cpp/include"
	}
	
	links {
		"AxionEngine"
	}

	prebuildcommands {
		"dotnet build \"%{wks.location}/AxionScripting/AxionScriptCore/AxionScriptCore.csproj\" -c %{cfg.buildcfg}"
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
		targetdir ("%{wks.location}/bin/" .. outputdir .. "/AxionStudio/RuntimeData")


	filter "configurations:Debug"
		postbuildcommands {
			"{COPY} " .. PhysXDir .. "/lib/debug/*.dll %{cfg.buildtarget.directory}",
			"{COPY} %{wks.location}/AxionEngineProject/AxionEngine/Vendor/dotnet/lib/nethost.dll %{cfg.buildtarget.directory}",
			"{COPY} %{wks.location}/AxionScripting/AxionScriptCore/bin/Debug/net10.0/AxionScriptCore.dll %{cfg.targetdir}",
			"{COPY} %{wks.location}/AxionScripting/AxionScriptCore/bin/Debug/net10.0/AxionScriptCore.runtimeconfig.json %{cfg.targetdir}"
		}

	filter "configurations:Release or Distribution"
		postbuildcommands {
			"{COPY} " .. PhysXDir .. "/lib/release/*.dll %{cfg.buildtarget.directory}",
			"{COPY} %{wks.location}/AxionEngineProject/AxionEngine/Vendor/dotnet/lib/nethost.dll %{cfg.buildtarget.directory}",
			"{COPY} %{wks.location}/AxionScripting/AxionScriptCore/bin/Release/net10.0/AxionScriptCore.dll %{cfg.targetdir}",
			"{COPY} %{wks.location}/AxionScripting/AxionScriptCore/bin/Release/net10.0/AxionScriptCore.runtimeconfig.json %{cfg.targetdir}"
		}
