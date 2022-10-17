#pragma once
namespace Hazel
{
	enum class AudioFileFormat : uint8_t
	{
		None = 0,
		MP3,
		OGG
	};

	enum class AudioSourceState : uint8_t
	{
		None = 0,
		Initial,
		Playing,
		Paused,
		Stopped,
	};

	namespace Utils
	{
		inline int AudioSourceStateToAlSourceState(AudioSourceState state)
		{
			// Values copied from al.h
			switch (state)
			{
			case AudioSourceState::Initial: return 0x1011;
			case AudioSourceState::Playing: return 0x1012;
			case AudioSourceState::Paused: return 0x1013;
			case AudioSourceState::Stopped: return 0x1014;
			default: return 0;
			}
		}

		inline AudioSourceState AlSourceStateToAudioSourceState(int state)
		{
			// Values copied from al.h
			switch (state)
			{
			case 0x1011: return AudioSourceState::Initial;
			case 0x1012: return AudioSourceState::Playing;
			case 0x1013: return AudioSourceState::Paused;
			case 0x1014: return AudioSourceState::Stopped;
			default: return AudioSourceState::None;
			}
		}
	}
}
