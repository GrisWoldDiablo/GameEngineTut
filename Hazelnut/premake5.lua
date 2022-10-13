project "Hazelnut"
	kind "ConsoleApp"
	language "C++"
	cppdialect "C++17"
	staticruntime "off"

	targetdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
	objdir ("%{wks.location}/bin-int/" .. outputdir .. "/%{prj.name}")

	files
	{
		"src/**.h",
		"src/**.cpp",
		"assets/**.*"
	}

	includedirs
	{
		"src",
		"%{wks.location}/Hazel/vendor/spdlog/include",
		"%{wks.location}/Hazel/src",
		"%{wks.location}/Hazel/vendor",
		"%{IncludeDir.glm}",
		"%{IncludeDir.entt}",
		"%{IncludeDir.ImGuizmo}",
		"%{IncludeDir.yaml_cpp}"
	}

	links
	{
		"Hazel"
	}

	filter "system:windows"
		systemversion "latest"

	filter "configurations:Debug"
		defines "HZ_DEBUG"
		runtime "Debug"
		symbols "on"

	filter "configurations:Release"
		defines "HZ_RELEASE"
		runtime "Release"
		optimize "on"

	filter "configurations:Dist"
		kind "WindowedApp"
		defines "HZ_DIST"
		runtime "Release"
		optimize "on"
