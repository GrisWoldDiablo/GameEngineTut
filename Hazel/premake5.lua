project "Hazel"
	kind "StaticLib"
	language "C++"
	cppdialect "C++20"
	staticruntime "off"

	targetdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
	objdir ("%{wks.location}/bin-int/" .. outputdir .. "/%{prj.name}")

	pchheader "hzpch.h"
	pchsource "src/hzpch.cpp"

	files
	{
		"src/**.h",
		"src/**.cpp",
		"vendor/stb_image/**.h",
		"vendor/stb_image/**.cpp",
		"vendor/glm/glm/**.hpp",
		"vendor/glm/glm/**.inl",
		"vendor/ImGuizmo/ImGuizmo.h",
		"vendor/ImGuizmo/ImGuizmo.cpp",
		"vendor/ImGui/misc/cpp/imgui_stdlib.h",
		"vendor/ImGui/misc/cpp/imgui_stdlib.cpp"
	}

	includedirs
	{
		"src",
		"vendor/spdlog/include",
		"%{IncludeDir.stb_image}",
		"%{IncludeDir.yaml_cpp}",
		"%{IncludeDir.Box2D}",
		"%{IncludeDir.GLFW}",
		"%{IncludeDir.Glad}",
		"%{IncludeDir.ImGui}",
		"%{IncludeDir.ImGuizmo}",
		"%{IncludeDir.glm}",
		"%{IncludeDir.entt}",
		"%{IncludeDir.mono}",
		"%{IncludeDir.filewatch}",
		"%{IncludeDir.VulkanSDK}",
		"%{IncludeDir.OpenALSoft}/include",
		"%{IncludeDir.OpenALSoft}/src",
		"%{IncludeDir.OpenALSoft}/src/common",
		"%{IncludeDir.OpenALSoft}/src/core",
		"%{IncludeDir.libogg}",
		"%{IncludeDir.Vorbis}",
		"%{IncludeDir.minimp3}",
		"%{IncludeDir.msdfgen}",
		"%{IncludeDir.msdf_atlas_gen}"
	}

	defines
	{
		"_CRT_SECURE_NO_WARNINGS",
		"GLFW_INCLUDE_NONE",
		"AL_LIBTYPE_STATIC",
		"RESTRICT=__restrict",
		"_SILENCE_ALL_CXX23_DEPRECATION_WARNINGS"
	}
	
	links
	{
		"Box2D",
		"GLFW",
		"Glad",
		"ImGui",
		"yaml-cpp",
		"opengl32.lib",
		"OpenAL-Soft",
		"Vorbis",
		"msdf-atlas-gen",

		"%{Library.mono}"
	}

	filter "files:vendor/ImGuizmo/**.cpp"
	flags { "NoPCH" }

	filter "files:vendor/ImGui/misc/cpp/**.cpp"
	flags { "NoPCH" }
	
	filter "system:windows"
		systemversion "latest"

		links
		{
			"%{Library.WinSock}",
			"%{Library.WinMM}",
			"%{Library.WinVersion}",
			"%{Library.BCrypt}"
		}

	filter "configurations:Debug"
		defines "HZ_DEBUG"
		runtime "Debug"
		symbols "on"

		links
		{
			"%{Library.ShaderC_Debug}",
			"%{Library.SPIRV_Cross_Debug}",
			"%{Library.SPIRV_Cross_GLSL_Debug}"
		}

	filter "configurations:Release"
		defines "HZ_RELEASE"
		runtime "Release"
		optimize "on"

		links
		{
			"%{Library.ShaderC_Release}",
			"%{Library.SPIRV_Cross_Release}",
			"%{Library.SPIRV_Cross_GLSL_Release}"
		}

	filter "configurations:Dist"
		defines "HZ_DIST"
		runtime "Release"
		optimize "on"

		links
		{
			"%{Library.ShaderC_Release}",
			"%{Library.SPIRV_Cross_Release}",
			"%{Library.SPIRV_Cross_GLSL_Release}",
			"%{Library.Msvcrt}"
		}
