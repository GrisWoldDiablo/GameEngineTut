#include "hzpch.h"
#include "AudioSource.h"
#include "AudioEngine.h"

#include "AL/alext.h"

namespace Hazel
{
	AudioSource::AudioSource(uint32_t alBuffer, const std::filesystem::path& filename, float lenght, AudioFileFormat fileFormat)
		: _alBuffer(alBuffer), _filePath(filename), _lenght(lenght), _fileFormat(fileFormat)
	{}

	AudioSource::~AudioSource()
	{
		alDeleteSources(1, &_alSource);
		alDeleteBuffers(1, &_alBuffer);
	}

	int AudioSource::GetState() const
	{
		ALenum state;
		alGetSourcei(_alSource, AL_SOURCE_STATE, &state);
		return state;
	}

	float AudioSource::GetOffset() const
	{
		ALfloat offset;
		alGetSourcef(_alSource, AL_SEC_OFFSET, &offset);
		return offset;
	}

	Ref<AudioSource> AudioSource::Create(const std::filesystem::path& filePath)
	{
		return AudioEngine::LoadAudioSource(filePath);
	}
}
