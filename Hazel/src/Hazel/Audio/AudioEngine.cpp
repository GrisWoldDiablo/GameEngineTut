#include "hzpch.h"
#include "AudioEngine.h"
#include "AudioTypes.h"
#include "AudioSource.h"

#include "Hazel/Core/Timer.h"

#include "alhelpers.h"
#include "AL/alext.h"
#include "alc/device.h"

#define MINIMP3_IMPLEMENTATION
#include "minimp3.h"
#include "minimp3_ex.h"

#define OV_EXCLUDE_STATIC_CALLBACKS
#include "vorbis/codec.h"
#include "vorbis/vorbisfile.h"

namespace Hazel
{
	namespace Utils
	{
		static AudioFileFormat GetAudioFileFormat(const std::filesystem::path& filePath)
		{
			auto extension = filePath.extension().string();

			if (extension == ".mp3")
			{
				return AudioFileFormat::MP3;
			}

			if (extension == ".ogg")
			{
				return AudioFileFormat::OGG;
			}

			return AudioFileFormat::None;
		}

		static ALenum GetOpenALFormat(uint32_t channels)
		{
			switch (channels)
			{
			case 1: return AL_FORMAT_MONO16;
			case 2: return AL_FORMAT_STEREO16;
			default:
				HZ_ASSERT(false, "Unsupported ALFormat for the amount of channels [{}]", channels);
				return AL_NONE;
				break;
			}
		}
	}

	struct AudioEngineData
	{
		ALCcontext* AudioContext = nullptr;
		ALCdevice* AudioDevice = nullptr;

		mp3dec_t Mp3d{};
		uint8_t* AudioScratchBuffer = nullptr;
		uint32_t AudioScratchBufferSize = 10 * 1024 * 1024;

		std::unordered_map<std::string, Ref<AudioSource>> UnassignedAudioSources;
		std::vector<Weak<AudioSource>> AssignedAudioSources;
	};

	static AudioEngineData* sAudioData = nullptr;

	void AudioEngine::Init()
	{
		sAudioData = new AudioEngineData();

		if (InitAL(nullptr, 0) != 0)
		{
			return;
		}

		sAudioData->AudioContext = alcGetCurrentContext();
		sAudioData->AudioDevice = alcGetContextsDevice(sAudioData->AudioContext);
		sAudioData->AudioScratchBuffer = new uint8_t[sAudioData->AudioScratchBufferSize];
		mp3dec_init(&sAudioData->Mp3d);

		PrintDeviceInfo();
	}

	void AudioEngine::Shutdown()
	{
		delete sAudioData;
		CloseAL();
	}

	Ref<AudioSource> AudioEngine::LoadAudioSource(const std::filesystem::path& filePath)
	{
		if (!std::filesystem::exists(filePath))
		{
			HZ_CORE_LERROR("[{0}] file not found", filePath.string());
			return nullptr;
		}

		Ref<AudioSource> newAudioSource = nullptr;

		switch (Utils::GetAudioFileFormat(filePath))
		{
		case AudioFileFormat::MP3:
			newAudioSource = LoadMP3(filePath);
			break;
		case AudioFileFormat::OGG:
			newAudioSource = LoadOgg(filePath);
			break;
		case AudioFileFormat::None:
		default:
			HZ_CORE_LERROR("No supported format for [{0}]", filePath.string());
			return nullptr;
		}

		sAudioData->AssignedAudioSources.push_back(newAudioSource);

		return newAudioSource;
	}

	void AudioEngine::ReleaseAudioSource(Ref<AudioSource> audioSource)
	{
		// Don't like much this release, need to revisit.
		// TODO setup a pool, Max size of map, might be the time for an asset manager.
		audioSource->Stop();
		audioSource->ResetFields();
		auto& sources = sAudioData->AssignedAudioSources;
		for (std::vector<Weak<AudioSource>>::iterator it = sources.begin(); it != sources.end();it++)
		{
			if (it->lock() == audioSource)
			{
				it = sources.erase(it);
				break;
			}
		}
		
		sAudioData->UnassignedAudioSources.emplace(audioSource->GetPath().string(), audioSource);
	}

	void AudioEngine::StopAllAudioSources()
	{
		// Todo keep track of which one is actually playing.
		for (auto& audioSource : sAudioData->AssignedAudioSources)
		{
			if (auto audioSourceRef = audioSource.lock())
			{
				audioSourceRef->Stop();
			}
		}
	}

	void AudioEngine::SetListenerPosition(const glm::vec3& position)
	{
		alListenerfv(AL_POSITION, &position.x);
	}

	bool AudioEngine::TryFindAudioSource(Ref<AudioSource>& audioSource, const std::filesystem::path& filePath)
	{
		auto foundPair = sAudioData->UnassignedAudioSources.find(filePath.string());
		if (foundPair != sAudioData->UnassignedAudioSources.end())
		{
			audioSource = foundPair->second;
			sAudioData->UnassignedAudioSources.erase(filePath.string());
			return true;
		}

		return false;
	}

	void AudioEngine::PrintDeviceInfo()
	{
		if (sAudioData->AudioDevice)
		{
			HZ_CORE_LINFO("Audio Device");
			HZ_CORE_LINFO("  Name: {0}", sAudioData->AudioDevice->DeviceName);
			HZ_CORE_LINFO("  Sample Rate: {0}", sAudioData->AudioDevice->Frequency);
			HZ_CORE_LINFO("  Max Sources: {0}", sAudioData->AudioDevice->SourcesMax);
			HZ_CORE_LINFO("    Mono: {0}", sAudioData->AudioDevice->NumMonoSources);
			HZ_CORE_LINFO("    Stereo: {0}", sAudioData->AudioDevice->NumStereoSources);
		}
	}

	Ref<AudioSource> AudioEngine::LoadMP3(const std::filesystem::path& filePath)
	{
		Timer timer;

		Ref<AudioSource> audioSource;
		if (TryFindAudioSource(audioSource, filePath))
		{
			return audioSource;
		}

		mp3dec_file_info_t info;
		int loadResult = mp3dec_load(&sAudioData->Mp3d, filePath.string().c_str(), &info, nullptr, nullptr);
		if (loadResult < 0)
		{
			HZ_CORE_LERROR("Failed to load mp3: [{0}]", filePath.string());
			return nullptr;
		}

		uint32_t size = static_cast<uint32_t>(info.samples * sizeof(mp3d_sample_t));

		auto sampleRate = info.hz;
		auto channels = info.channels;
		ALenum alFormat = Utils::GetOpenALFormat(channels);
		// TODO investigate, this is not giving an accurate length.
		float lenghtSeconds = size / (info.avg_bitrate_kbps * 1024.0f);

		ALuint alBuffer;
		alGenBuffers(1, &alBuffer);
		alBufferData(alBuffer, alFormat, info.buffer, size, sampleRate);

		audioSource = CreateRef<AudioSource>(alBuffer, filePath, lenghtSeconds, AudioFileFormat::MP3);
		alGenSources(1, &audioSource->_alSource);
		alSourcei(audioSource->_alSource, AL_BUFFER, alBuffer);

		if (alGetError() != AL_NO_ERROR)
		{
			HZ_CORE_LERROR("OpenAl-Soft failed to create the source: [{0}]", filePath.string());
			return nullptr;
		}

		HZ_CORE_LINFO("MP3 loading took {0}ms for [{1}]", timer.ElapsedMillis(), filePath.filename().string());

		return audioSource;
	}

	Ref<AudioSource> AudioEngine::LoadOgg(const std::filesystem::path& filePath)
	{
		Timer timer;

		Ref<AudioSource> audioSource;
		if (TryFindAudioSource(audioSource, filePath))
		{
			return audioSource;
		}

		FILE* file = fopen(filePath.string().c_str(), "rb");
		OggVorbis_File vorbisFile;

		int loadResult = ov_open(file, &vorbisFile, NULL, 0);
		if (loadResult < 0)
		{
			HZ_CORE_LERROR("OpenAl-Soft failed to open: [{0}]", filePath.string());
			if (file)
			{
				fclose(file);
			}
			return nullptr;
		}

		vorbis_info* vi = ov_info(&vorbisFile, -1);
		auto sampleRate = vi->rate;
		auto channels = vi->channels;
		ALenum alFormat = Utils::GetOpenALFormat(channels);

		uint64_t samples = ov_pcm_total(&vorbisFile, -1);
		float lenghtSeconds = static_cast<float>(samples) / static_cast<float>(sampleRate);
		uint32_t requiredBufferSize = static_cast<uint32_t>(2.0f * channels * samples);

		if (sAudioData->AudioScratchBufferSize < requiredBufferSize)
		{
			sAudioData->AudioScratchBufferSize = requiredBufferSize;
			delete[] sAudioData->AudioScratchBuffer;
			sAudioData->AudioScratchBuffer = new uint8_t[sAudioData->AudioScratchBufferSize];
		}

		uint8_t* oggBuffer = sAudioData->AudioScratchBuffer;
		uint8_t* bufferPtr = oggBuffer;

		bool hasReadingFailed = false;
		while (true)
		{
			int currentSection;
			long lenght = ov_read(&vorbisFile, (char*)bufferPtr, 4096, 0, 2, 1, &currentSection);

			if (lenght == 0)
			{
				break;
			}

			if (lenght < 0)
			{
				// Log error
				HZ_CORE_LERROR("OpenAl-Soft failed to read: [{0}]", filePath.string());
				hasReadingFailed = true;
				break;
			}

			bufferPtr += lenght;
		}

		uint32_t size = static_cast<uint32_t>(bufferPtr - oggBuffer);
		ov_clear(&vorbisFile);
		fclose(file);

		if (hasReadingFailed)
		{
			return nullptr;
		}

		ALuint alBuffer;
		alGenBuffers(1, &alBuffer);
		alBufferData(alBuffer, alFormat, oggBuffer, size, sampleRate);

		audioSource = CreateRef<AudioSource>(alBuffer, filePath, lenghtSeconds, AudioFileFormat::OGG);
		alGenSources(1, &audioSource->_alSource);
		alSourcei(audioSource->_alSource, AL_BUFFER, alBuffer);

		if (alGetError() != AL_NO_ERROR)
		{
			HZ_CORE_LERROR("OpenAl-Soft failed to create the source: [{0}]", filePath.string());
			return nullptr;
		}

		HZ_CORE_LINFO("OGG loading took {0}ms for [{1}]", timer.ElapsedMillis(), filePath.filename().string());


		return audioSource;
	}
}
