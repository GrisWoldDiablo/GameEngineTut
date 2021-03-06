#include "hzpch.h"
#include "Hazel/Utils/PlatformUtils.h"

#include <commdlg.h>
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>
#include "Hazel/Core/Application.h"

namespace Hazel
{
	bool FileDialogs::NewFile()
	{
		auto result = MessageBoxA
		(
			glfwGetWin32Window((GLFWwindow*)Application::Get().GetWindow().GetNativeWindow()),
			(LPCSTR)"You will lose current scene's unsaved progress.\nDo you want to continue?",
			(LPCSTR)"Warning!",
			MB_ICONWARNING | MB_YESNO
		);

		return result == IDYES;
	}

	std::string FileDialogs::OpenFile(const char* filter)
	{
		OPENFILENAMEA ofn;
		CHAR szFile[260] = { 0 };

		// Initialize OPENFILENAME
		ZeroMemory(&ofn, sizeof(OPENFILENAME));
		ofn.lStructSize = sizeof(OPENFILENAME);
		ofn.hwndOwner = glfwGetWin32Window((GLFWwindow*)Application::Get().GetWindow().GetNativeWindow());
		ofn.lpstrFile = szFile;
		ofn.nMaxFile = sizeof(szFile);
		ofn.lpstrFilter = filter;
		ofn.nFilterIndex = 1;
		ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;

		if (GetOpenFileNameA(&ofn) == TRUE)
		{
			return ofn.lpstrFile;
		}


		return std::string();
	}

	std::string FileDialogs::SaveFile(const char* filter, const char* defaultFileName)
	{
		OPENFILENAMEA ofn;
		CHAR szFile[260] = { 0 };

		if (defaultFileName != nullptr)
		{
			std::stringstream ss;
			ss << defaultFileName << std::strchr(filter, '\0');
			std::strcpy(szFile, ss.str().c_str());
		}

		// Initialize OPENFILENAME
		ZeroMemory(&ofn, sizeof(OPENFILENAME));
		ofn.lStructSize = sizeof(OPENFILENAME);
		ofn.hwndOwner = glfwGetWin32Window((GLFWwindow*)Application::Get().GetWindow().GetNativeWindow());
		ofn.lpstrFile = szFile;
		ofn.nMaxFile = sizeof(szFile);
		ofn.lpstrFilter = filter;
		ofn.lpstrDefExt = std::strchr(filter, '\0') + 1;
		ofn.nFilterIndex = 1;
		ofn.Flags = OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT | OFN_NOCHANGEDIR;

		if (GetSaveFileNameA(&ofn) == TRUE)
		{
			return ofn.lpstrFile;
		}

		return std::string();
	}
}
