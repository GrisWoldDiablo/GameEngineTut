#pragma once

#include <string>

namespace Hazel
{
	class FileDialogs
	{
	public:
		// Returns false if canceled.
		static bool NewFile();
		// Returns empty string if canceled.
		static std::string OpenFile(const char* filter);
		// Returns empty string if canceled.
		static std::string SaveFile(const char* filter);
	};
}
