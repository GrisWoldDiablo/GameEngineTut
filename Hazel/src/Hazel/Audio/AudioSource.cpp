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

	void AudioSource::Play()
	{
		AudioEngine::Play(shared_from_this());
	}

	void AudioSource::Stop()
	{
		AudioEngine::Stop(shared_from_this());
	}

	void AudioSource::Pause()
	{
		AudioEngine::Pause(shared_from_this());
	}

	void AudioSource::Rewind()
	{
		AudioEngine::Pause(shared_from_this());
	}

	AudioSourceState AudioSource::GetState() const
	{
		ALenum state;
		alGetSourcei(_alSource, AL_SOURCE_STATE, &state);
		return static_cast<AudioSourceState>(state);
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
