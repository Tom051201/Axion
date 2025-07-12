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

group "Dependencies"
	include "AxionEngineProject/AxionEngine/Vendor/glad"
	include "AxionEngineProject/AxionEngine/Vendor/imgui"

group ""

include "AxionEngineProject"
include "AxionStudioProject"
include "SandboxProject"
