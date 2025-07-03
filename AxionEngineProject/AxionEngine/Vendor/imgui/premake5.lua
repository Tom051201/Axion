project "ImGui"
	location "AxionEngineProject/AxionEngine/Vendor/imgui"
	kind "StaticLib"
	language "C++"
	cppdialect "C++17"
	staticruntime "On"

	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

	files {
		"AxionEngineProject/AxionEngine/Vendor/imgui/imconfig.h",
		"AxionEngineProject/AxionEngine/Vendor/imgui/imgui.h",
		"AxionEngineProject/AxionEngine/Vendor/imgui/imgui.cpp",
		"AxionEngineProject/AxionEngine/Vendor/imgui/imgui_draw.cpp",
		"AxionEngineProject/AxionEngine/Vendor/imgui/imgui_internal.h",
		"AxionEngineProject/AxionEngine/Vendor/imgui/imgui_widgets.cpp",
		"AxionEngineProject/AxionEngine/Vendor/imgui/imstb_rectpack.h",
		"AxionEngineProject/AxionEngine/Vendor/imgui/imstb_textedit.h",
		"AxionEngineProject/AxionEngine/Vendor/imgui/imstb_truetype.h",
		"AxionEngineProject/AxionEngine/Vendor/imgui/imgui_demo.cpp",
		"AxionEngineProject/AxionEngine/Vendor/imgui/imgui_tables.cpp",
		"AxionEngineProject/AxionEngine/Vendor/imgui/backends/imgui_impl_win32.h",
		"AxionEngineProject/AxionEngine/Vendor/imgui/backends/imgui_impl_dx12.h",
		"AxionEngineProject/AxionEngine/Vendor/imgui/backends/imgui_impl_win32.cpp",
		"AxionEngineProject/AxionEngine/Vendor/imgui/backends/imgui_impl_dx12.cpp"
	}
	includedirs {
		"AxionEngineProject/AxionEngine/Vendor/imgui"
	}
	filter "system:windows"
		systemversion "latest"

	filter "configurations:Debug"
		runtime "Debug"
		symbols "on"

	filter "configurations:Release"
		runtime "Release"
		optimize "on"
