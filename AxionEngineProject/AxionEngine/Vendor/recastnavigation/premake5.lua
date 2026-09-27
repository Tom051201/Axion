project "RecastNavigation"
	kind "StaticLib"
	language "C++"
	cppdialect "C++20"
	staticruntime "On"

	targetdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
	objdir ("%{wks.location}/bin-int/" .. outputdir .. "/%{prj.name}")

	files {
		"Recast/Source/**.cpp",
		"Recast/Include/**.h",
		"Detour/Source/**.cpp",
		"Detour/Include/**.h"
	}

	includedirs {
		"Recast/Include",
		"Detour/Include"
	}

	filter "configurations:Debug"
		symbols "on"

	filter "configurations:Release"
		optimize "on"
