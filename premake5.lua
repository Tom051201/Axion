workspace "AxionEngineWorkspace"
	architecture "x64"
	startproject "AxionStudio"
	configurations {
		"Debug",
		"Release",
		"Distribution"
	}



outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

IncludeDir = {}
IncludeDir["ImGui"] = "%{wks.location}/AxionEngineProject/AxionEngine/Vendor/imgui"
IncludeDir["GLAD"] = "%{wks.location}/AxionEngineProject/AxionEngine/Vendor/glad"
IncludeDir["yaml_cpp"] = "%{wks.location}/AxionEngineProject/AxionEngine/Vendor/yaml-cpp"

group "Dependencies"
	include "AxionEngineProject/AxionEngine/Vendor/glad"
	include "AxionEngineProject/AxionEngine/Vendor/imgui"
	include "AxionEngineProject/AxionEngine/Vendor/yaml-cpp"

group ""

include "AxionEngineProject"
include "AxionStudioProject"
include "SandboxProject"
