#include "hzpch.h"
#include "AudioSource.h"
#include "AudioEngine.h"

#include "Hazel/Math/HMath.h"

#include "glm/gtc/type_ptr.hpp"

#include "AL/alext.h"

namespace Hazel
{
	Ref<AudioSource> AudioSource::Create(const std::filesystem::path& path)
	{
		return AudioEngine::LoadAudioSource(path);
	}

	AudioSource::AudioSource(uint32_t alBuffer, const std::filesystem::path& path, float length, AudioFileFormat fileFormat)
		: _alBuffer(alBuffer), _path(path), _length(length), _fileFormat(fileFormat)
	{}

	AudioSource::~AudioSource()
	{
		AudioEngine::ReleaseALSource(_alSource);
		alDeleteSources(1, &_alSource);
		alDeleteBuffers(1, &_alBuffer);
	}

	void AudioSource::Play()
	{
		alSourcePlay(_alSource);
	}

	void AudioSource::Stop()
	{
		alSourceStop(_alSource);
	}

	void AudioSource::Pause()
	{
		alSourcePause(_alSource);
	}

	void AudioSource::Rewind()
	{
		alSourceRewind(_alSource);
	}

	AudioSourceState AudioSource::GetState()
	{
		ALenum state;
		alGetSourcei(_alSource, AL_SOURCE_STATE, &state);
		return Utils::AlSourceStateToAudioSourceState(state);
	}

	float AudioSource::GetOffset()
	{
		ALfloat offset;
		alGetSourcef(_alSource, AL_SEC_OFFSET, &offset);
		return offset;
	}

	void AudioSource::SetOffset(float offset)
	{
		if (offset >= 0.0f)
		{
			alSourcef(_alSource, AL_SEC_OFFSET, offset);
		}
	}

	void AudioSource::SetGain(float gain)
	{
		if (gain >= 0.0f && !HMath::IsNearlyEqual(_gain, gain))
		{
			_gain = gain;
			alSourcef(_alSource, AL_GAIN, gain);
		}
	}

	void AudioSource::SetPitch(float pitch)
	{
		if (pitch >= 0.0f && !HMath::IsNearlyEqual(_pitch, pitch))
		{
			_pitch = pitch;
			alSourcef(_alSource, AL_PITCH, pitch);
		}
	}

	void AudioSource::SetLoop(bool isLoop)
	{
		if (_isLoop != isLoop)
		{
			_isLoop = isLoop;
			alSourcei(_alSource, AL_LOOPING, _isLoop ? AL_TRUE : AL_FALSE);
		}
	}

	void AudioSource::Set3D(bool is3D)
	{
		if (_is3D != is3D)
		{
			_is3D = is3D;
			alSourcei(_alSource, AL_SOURCE_SPATIALIZE_SOFT, _is3D ? AL_TRUE : AL_FALSE);
		}
	}

	void AudioSource::SetPosition(const glm::vec3& position)
	{
		if (!HMath::IsNearlyEqual(_position, position))
		{
			_position = position;
			alSourcefv(_alSource, AL_POSITION, glm::value_ptr(_position));
		}
	}

	void AudioSource::ResetFields()
	{
		_gain = 1.0f;
		_pitch = 1.0f;
		_is3D = false;
		_isLoop = false;
		_position = { 0.0f, 0.0f, 0.0f };
	}
}
