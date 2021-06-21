#pragma once

#include <string>

namespace Hazel
{
	class FileDialogs
	{
	public:
		// Returns empty string if canceled.
		static std::string OpenFile(const char* filter);
		// Returns empty string if canceled.
		static std::string SaveFile(const char* filter);
	};
}
