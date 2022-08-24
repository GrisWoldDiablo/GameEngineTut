#pragma once
#include "AudioEnum.h"

namespace Hazel
{
	class AudioSource;

	class AudioEngine
	{
	public:
		static void Init();
		static void Shutdown();

		static Ref<AudioSource> LoadAudioSource(const std::filesystem::path& filePath);
		static void Play(const Ref<AudioSource>& audioSource);
		static void Stop(const Ref<AudioSource>& audioSource);
		static void Pause(const Ref<AudioSource>& audioSource);
		static void Rewind(const Ref<AudioSource>& audioSource);
		static AudioSourceState GetState(const Ref<AudioSource>& audioSource);
		static float GetOffset(const Ref<AudioSource>& audioSource);
		static void SetOffset(const Ref<AudioSource>& audioSource, float offset);

	private:
		static void PrintDeviceInfo();
		static Ref<AudioSource> LoadMP3(const std::filesystem::path& filePath);
		static Ref<AudioSource> LoadOgg(const std::filesystem::path& filePath);
	};
}
