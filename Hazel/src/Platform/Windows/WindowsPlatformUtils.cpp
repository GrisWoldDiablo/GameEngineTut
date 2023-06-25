#include "hzpch.h"
#include "Hazel/Utils/PlatformUtils.h"
#include "Hazel/Core/Application.h"

#include <shlobj_core.h>

#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

namespace Hazel
{
	namespace Utils
	{
		bool Path::IsSubpath(const std::filesystem::path& subpath, const std::filesystem::path& basepath)
		{
			const auto relativePath = std::filesystem::relative(subpath, basepath);
			return !relativePath.empty() && relativePath.native()[0] != '.';
		}
	}

	void FileDialogs::MessagePopup(const std::string& message, const std::string& title)
	{
		std::thread([](const std::string& message, const std::string& title)
		{
			MessageBoxA
			(
				glfwGetWin32Window(static_cast<GLFWwindow*>(Application::Get().GetWindow().GetNativeWindow())),
				message.c_str(),
				title.c_str(),
				MB_ICONWARNING | MB_OK
			);
		}, message, title).detach();
	}

	bool FileDialogs::QuestionPopup(const char* message, const char* title)
	{
		const auto result = MessageBoxA
		(
			glfwGetWin32Window(static_cast<GLFWwindow*>(Application::Get().GetWindow().GetNativeWindow())),
			message,
			title,
			MB_ICONINFORMATION | MB_YESNO
		);

		return result == IDYES;
	}

	std::string FileDialogs::OpenFile(const char* filter)
	{
		OPENFILENAMEA ofn;
		CHAR szFile[260] = {0};

		// Initialize OPENFILENAME
		ZeroMemory(&ofn, sizeof(OPENFILENAME));
		ofn.lStructSize = sizeof(OPENFILENAME);
		ofn.hwndOwner = glfwGetWin32Window(static_cast<GLFWwindow*>(Application::Get().GetWindow().GetNativeWindow()));
		ofn.lpstrFile = szFile;
		ofn.nMaxFile = sizeof(szFile);
		ofn.lpstrFilter = filter;
		ofn.nFilterIndex = 1;
		ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_READONLY | OFN_NOCHANGEDIR;

		if (GetOpenFileNameA(&ofn))
		{
			return ofn.lpstrFile;
		}


		return {};
	}

	std::string FileDialogs::SaveFile(const char* filter, const char* defaultFileName, const std::filesystem::path& defaultPath)
	{
		OPENFILENAMEA ofn;
		CHAR szFile[260] = {0};

		if (defaultFileName != nullptr)
		{
			const auto path = defaultPath / defaultFileName;
			std::stringstream ss;
			ss << path.string().c_str() << std::strchr(filter, '\0');
			std::strcpy(szFile, ss.str().c_str());
		}

		// Initialize OPENFILENAME
		ZeroMemory(&ofn, sizeof(OPENFILENAMEA));
		ofn.lStructSize = sizeof(OPENFILENAMEA);
		ofn.lpstrInitialDir = defaultPath.string().c_str();
		ofn.hwndOwner = glfwGetWin32Window(static_cast<GLFWwindow*>(Application::Get().GetWindow().GetNativeWindow()));
		ofn.lpstrFile = szFile;
		ofn.nMaxFile = sizeof(szFile);
		ofn.lpstrFilter = filter;
		ofn.lpstrDefExt = std::strchr(filter, '\0') + 1;
		ofn.nFilterIndex = 1;
		ofn.Flags = OFN_OVERWRITEPROMPT | OFN_NOCHANGEDIR;

		if (GetSaveFileNameA(&ofn))
		{
			return ofn.lpstrFile;
		}

		return {};
	}

	std::filesystem::path FileDialogs::SelectFolder(const std::filesystem::path& rootPath)
	{
		ITEMIDLIST* pidlRoot = nullptr;
		if (!rootPath.empty())
		{
			SHParseDisplayName(rootPath.c_str(), nullptr, &pidlRoot, 0, nullptr);
		}

		BROWSEINFOA bi;
		char path[MAX_PATH + 1];
		bi.hwndOwner = glfwGetWin32Window(static_cast<GLFWwindow*>(Application::Get().GetWindow().GetNativeWindow()));
		bi.pidlRoot = pidlRoot;
		bi.pszDisplayName = path;
		bi.lpszTitle = "Choose New Project's Folder";
		bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE | BIF_STATUSTEXT;
		bi.lpfn = nullptr;
		bi.lParam = 0;

		const LPITEMIDLIST pidl = SHBrowseForFolderA(&bi);
		if (pidl != nullptr && SHGetPathFromIDListA(pidl, path))
		{
			return path;
		}

		return {};
	}

	void FileDialogs::ExecuteOpenFile(const char* filePath)
	{
		const auto hwnd = glfwGetWin32Window(static_cast<GLFWwindow*>(Application::Get().GetWindow().GetNativeWindow()));
		ShellExecuteA(hwnd, nullptr, filePath, nullptr, nullptr, SW_SHOW);
	}
}
