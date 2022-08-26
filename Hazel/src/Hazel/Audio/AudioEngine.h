#pragma once

namespace Hazel
{
	class AudioSource;

	class AudioEngine
	{
	public:
		static void Init();
		static void Shutdown();

		static Ref<AudioSource> LoadAudioSource(const std::filesystem::path& filePath);
		static void ReleaseAudioSource(Ref<AudioSource> audioSource);

	private:
		static bool TryFindAudioSource(Ref<AudioSource>& audioSource, const std::filesystem::path& filePath);

		static void PrintDeviceInfo();
		static Ref<AudioSource> LoadMP3(const std::filesystem::path& filePath);
		static Ref<AudioSource> LoadOgg(const std::filesystem::path& filePath);
	};
}
