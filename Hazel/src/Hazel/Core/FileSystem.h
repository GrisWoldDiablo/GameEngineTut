#pragma once

#include "Buffer.h"

namespace Hazel
{
	class FileSystem
	{
	public:
		static Buffer ReadFileBinary(const std::filesystem::path& filePath);
		static bool ReplaceInFile(const std::filesystem::path& inputFilePath, const std::filesystem::path& outputFilePath, const std::string& toReplace, const std::string& replacement);

		static const std::filesystem::path& GetApplicationPath() { return _sApplicationRootPath; }

	private:
		static std::filesystem::path _sApplicationRootPath;
	};
}
