#include "hzpch.h"
#include "FileSystem.h"

namespace Hazel
{
	std::filesystem::path FileSystem::_sApplicationRootPath = std::filesystem::current_path();

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

	bool FileSystem::ReplaceInFile(const std::filesystem::path& inputFilePath, const std::filesystem::path& outputFilePath, const std::string& toReplace, const std::string& replacement)
	{
		std::ifstream inputStream(inputFilePath);

		if (!inputStream)
		{
			HZ_CORE_LERROR("Fail to load input FilePath {0}!", inputFilePath);
			return false;
		}

		if (std::filesystem::exists(outputFilePath))
		{
			HZ_CORE_LERROR("Output FilePath already exist {0}", outputFilePath);
			return false;
		}

		std::ofstream outputStream(outputFilePath);
		if (!outputStream)
		{
			HZ_CORE_LERROR("Fail to create output FilePath {0}!", outputFilePath);
			return false;
		}

		std::string line;
		while (std::getline(inputStream, line))
		{
			const std::size_t pos = line.find(toReplace);
			if (pos != std::string::npos)
			{
				line.replace(pos, toReplace.length(), replacement);
			}

			outputStream << line << std::endl;
		}

		inputStream.close();
		outputStream.close();
		return true;
	}
}
