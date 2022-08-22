#pragma once

#include "AudioFileFormat.h"

namespace Hazel
{
	class AudioSource
	{
	public:
		AudioSource() = default;
		AudioSource(uint32_t alBuffer, const std::filesystem::path& filePath, float lenght, AudioFileFormat fileFormat);
		~AudioSource();

		int GetState() const;
		float GetOffset() const;
		const std::filesystem::path& GetFilePath() const { return _filePath; }
		float GetLenght() const { return _lenght; }

		static Ref<AudioSource> Create(const std::filesystem::path& filePath);

	private:
		uint32_t _alBuffer = 0;
		uint32_t _alSource = 0;

		std::filesystem::path _filePath;
		float _lenght;
		AudioFileFormat _fileFormat;

		friend class AudioEngine;
	};
}
