project "ImGui"
	kind "StaticLib"
	language "C++"
	cppdialect "C++17"
	staticruntime "On"

	targetdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
	objdir ("%{wks.location}/bin-int/" .. outputdir .. "/%{prj.name}")

	files {
		"imconfig.h",
		"imgui.h",
		"imgui.cpp",
		"imgui_draw.cpp",
		"imgui_internal.h",
		"imgui_widgets.cpp",
		"imstb_rectpack.h",
		"imstb_textedit.h",
		"imstb_truetype.h",
		"imgui_demo.cpp",
		"imgui_tables.cpp",
		"backends/imgui_impl_win32.h",
		"backends/imgui_impl_dx12.h",
		"backends/imgui_impl_opengl3.h",
		"backends/imgui_impl_opengl3_loader.h",
		"backends/imgui_impl_win32.cpp",
		"backends/imgui_impl_dx12.cpp",
		"backends/imgui_impl_opengl3.cpp"
	}

	includedirs {
		"."
	}

	filter "system:windows"
		systemversion "latest"

	filter "configurations:Debug"
		runtime "Debug"
		symbols "on"

	filter "configurations:Release"
		runtime "Release"
		optimize "on"
