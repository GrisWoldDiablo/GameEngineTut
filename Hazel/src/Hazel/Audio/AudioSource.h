#pragma once

#include "AudioEnum.h"

namespace Hazel
{
	class AudioSource : public std::enable_shared_from_this<AudioSource>
	{
	public:
		AudioSource() = default;
		AudioSource(uint32_t alBuffer, const std::filesystem::path& path, float length, AudioFileFormat fileFormat);
		~AudioSource();

		void Play();
		void Stop();
		void Pause();
		void Rewind();

		AudioSourceState GetState();
		float GetOffset();
		void SetOffset(float offset);
		const std::filesystem::path& GetPath() const { return _path; }
		float GetLength() const { return _length; }

		static Ref<AudioSource> Create(const std::filesystem::path& path);

	private:
		uint32_t _alBuffer = 0;
		uint32_t _alSource = 0;

		std::filesystem::path _path;
		float _length;
		AudioFileFormat _fileFormat;

		friend class AudioEngine;
	};
}
