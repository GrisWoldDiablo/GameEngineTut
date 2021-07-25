#pragma once

#include <string>

namespace Hazel
{
	class FileDialogs
	{
	public:
		// Message Box with yes and no question.
		static bool QuestionBox(const char* message, const char* title);
		// Returns empty string if canceled.
		static std::string OpenFile(const char* filter);
		// Returns empty string if canceled.
		static std::string SaveFile(const char* filter, const char* defaultFileName = nullptr);

		static void ExecuteFile(const char* filePath);
	};
}
