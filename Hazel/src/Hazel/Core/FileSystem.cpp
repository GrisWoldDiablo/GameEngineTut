#include "hzpch.h"
#include "FileSystem.h"

namespace Hazel
{
	Buffer FileSystem::ReadFileBinary(const std::filesystem::path& filePath)
	{
		std::ifstream stream(filePath, std::ios::binary | std::ios::ate);

		if (!stream)
		{
			HZ_CORE_LERROR("Fail to load filePath {0}!", filePath);
			return {};
		}

		const std::streampos end = stream.tellg();
		stream.seekg(0, std::ios::beg);
		const auto size = static_cast<uint32_t>(end - stream.tellg());

		if (size == 0)
		{
			HZ_CORE_LERROR("File empty!");
			return {};
		}

		Buffer buffer(size);
		stream.read(buffer.As<char>(), size);
		stream.close();

		return buffer;
	}
}
