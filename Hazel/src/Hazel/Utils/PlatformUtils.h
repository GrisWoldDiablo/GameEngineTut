#pragma once

#include <string>

namespace Hazel
{
	namespace Utils
	{
		class Path
		{
		public:
			static bool IsSubpath(const std::filesystem::path& subpath, const std::filesystem::path& basepath);
		};
	}

	class FileDialogs
	{
	public:
		// Message Box with ok.
		static void MessagePopup(const std::string& message, const std::string& title);
		// Message Box with yes and no question.
		static bool QuestionPopup(const char* message, const char* title);
		// Returns empty string if canceled.
		static std::string OpenFile(const char* filter);
		// Returns empty string if canceled.
		static std::string SaveFile(const char* filter, const char* defaultFileName = nullptr, const std::filesystem::path& defaultPath = {});

		static std::filesystem::path SelectFolder(const std::filesystem::path& rootPath = {});

		// Execute the filePath 
		static void ExecuteOpenFile(const char* filePath);
	};
}
