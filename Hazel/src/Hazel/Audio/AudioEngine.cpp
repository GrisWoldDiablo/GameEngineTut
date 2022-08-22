#include "hzpch.h"
#include "AudioEngine.h"
#include "AudioFileFormat.h"
#include "AudioSource.h"

#include "alhelpers.h"
#include "AL/alext.h"
#include "alc/device.h"

#define MINIMP3_IMPLEMENTATION
#include "minimp3.h"
#include "minimp3_ex.h"

#define OV_EXCLUDE_STATIC_CALLBACKS
#include "vorbis/codec.h"
#include "vorbis/vorbisfile.h"

// TODO remove this is only for Audio testing
#include <thread>		
#include <chrono>

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

			return AudioFileFormat::NONE;
		}

		static ALenum GetOpenALFormat(uint32_t channels)
		{
			switch (channels)
			{
			case 1: return AL_FORMAT_MONO16;
			case 2: return AL_FORMAT_STEREO16;
			default:
				HZ_ASSERT(false, "Unsupported ALFormat for the amount of channels [{}]", channels);
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
	};

	static AudioEngineData* sAudioData = nullptr;

	static Ref<AudioSource> test1;
	static Ref<AudioSource> test2;

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

		// TODO remove this is only for Audio testing
		test1 = LoadAudioSource("assets/audio/rocavaco-Bass17w19.mp3");
		Play(test1);

		test2 = LoadAudioSource("assets/audio/Splash.ogg");
		Play(test2);
		do 
		{
			HZ_CORE_LINFO("{0}", test2->GetOffset());
			using namespace std::literals::chrono_literals;
			std::this_thread::sleep_for(5ms);
		} while (test2->GetState() != AL_STOPPED);
	}

	void AudioEngine::Shutdown()
	{
		// TODO remove this is only for Audio testing
		test1.reset();
		test2.reset();
		CloseAL();
		delete sAudioData;
	}

	Ref<AudioSource> AudioEngine::LoadAudioSource(const std::filesystem::path& filePath)
	{
		switch (Utils::GetAudioFileFormat(filePath))
		{
		case AudioFileFormat::MP3:	
			return LoadMP3(filePath);
		case AudioFileFormat::OGG:	
			return LoadOgg(filePath);
		case AudioFileFormat::NONE:
		default:
			HZ_CORE_LERROR("No supported format for [{0}]", filePath.string());
			return nullptr;
		}
	}

	void AudioEngine::Play(const Ref<AudioSource>& audioSource)
	{
		HZ_ASSERT(audioSource, "Audio Source is Invalid!");

		if (audioSource)
		{
			alSourcePlay(audioSource->_alSource);
		}
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
		float lenghtSeconds = size / (info.avg_bitrate_kbps * 1024.0f);

		ALuint alBuffer;
		alGenBuffers(1, &alBuffer);
		alBufferData(alBuffer, alFormat, info.buffer, size, sampleRate);

		auto audioSource = CreateRef<AudioSource>(alBuffer, filePath, lenghtSeconds, AudioFileFormat::MP3);
		alGenSources(1, &audioSource->_alSource);
		alSourcei(audioSource->_alSource, AL_BUFFER, alBuffer);

		if (alGetError() != AL_NO_ERROR)
		{
			HZ_CORE_LERROR("OpenAl-Soft failed to create the source: [{0}]", filePath.string());
			return nullptr;
		}

		return audioSource;
	}

	Ref<AudioSource> AudioEngine::LoadOgg(const std::filesystem::path& filePath)
	{
		FILE* file = fopen(filePath.string().c_str(), "rb");
		OggVorbis_File vorbisFile;

		int loadResult = ov_open(file, &vorbisFile, NULL, 0);
		if (loadResult < 0)
		{
			HZ_CORE_LERROR("OpenAl-Soft failed to open: [{0}]", filePath.string());
			fclose(file);
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
			long lenght = ov_read(&vorbisFile, (char*)bufferPtr, sizeof(bufferPtr), 0, 2, 1, &currentSection);

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

		auto audioSource = CreateRef<AudioSource>(alBuffer, filePath, lenghtSeconds, AudioFileFormat::OGG);
		alGenSources(1, &audioSource->_alSource);
		alSourcei(audioSource->_alSource, AL_BUFFER, alBuffer);

		if (alGetError() != AL_NO_ERROR)
		{
			HZ_CORE_LERROR("OpenAl-Soft failed to create the source: [{0}]", filePath.string());
			return nullptr;
		}

		return audioSource;
	}
}
