#include "hzpch.h"
#include "AudioSource.h"
#include "AudioEngine.h"

#include "AL/alext.h"

namespace Hazel
{
	AudioSource::AudioSource(uint32_t alBuffer, const std::filesystem::path& path, float length, AudioFileFormat fileFormat)
		: _alBuffer(alBuffer), _path(path), _length(length), _fileFormat(fileFormat)
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

	AudioSourceState AudioSource::GetState()
	{
		return AudioEngine::GetState(shared_from_this());
	}

	float AudioSource::GetOffset()
	{
		return AudioEngine::GetOffset(shared_from_this());
	}

	void AudioSource::SetOffset(float offset)
	{
		AudioEngine::SetOffset(shared_from_this(), offset);
	}

	Ref<AudioSource> AudioSource::Create(const std::filesystem::path& path)
	{
		return AudioEngine::LoadAudioSource(path);
	}
}
