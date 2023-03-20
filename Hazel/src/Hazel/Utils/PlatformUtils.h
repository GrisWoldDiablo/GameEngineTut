#pragma once

#include <string>

namespace Hazel
{
	class FileDialogs
	{
	public:
		// Message Box with ok.
		static void MessagePopup(const char* message, const char* title);
		// Message Box with yes and no question.
		static bool QuestionPopup(const char* message, const char* title);
		// Returns empty string if canceled.
		static std::string OpenFile(const char* filter);
		// Returns empty string if canceled.
		static std::string SaveFile(const char* filter, const char* defaultFileName = nullptr, const std::filesystem::path& defaultPath = {});

		static std::filesystem::path SelectFolder(const std::filesystem::path& rootPath);

		// Execute the filePath 
		static void ExecuteOpenFile(const char* filePath);
	};
}
