#pragma once

#include "Buffer.h"

namespace Hazel
{
	class FileSystem
	{
	public:
		static Buffer ReadFileBinary(const std::filesystem::path& filePath);
	};
}
