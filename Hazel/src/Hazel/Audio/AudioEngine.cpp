#include "hzpch.h"
#include "AudioEngine.h"
#include "alhelpers.h"

#include "AL/alc.h"

#define MINIMP3_IMPLEMENTATION
#include "minimp3.h"
#include "minimp3_ex.h"

#define OV_EXCLUDE_STATIC_CALLBACKS
#include "vorbis/codec.h"
#include "vorbis/vorbisfile.h"

namespace Hazel
{
	struct AudioEngineData
	{
		ALCdevice* AudioDevice = nullptr;
		mp3dec_t Mp3d;
		uint8_t* AudioScratchBuffer = nullptr;
		uint32_t AudioScratchBufferSize = 10 * 1024 * 1024;
	};

	static AudioEngineData* sAudioData = nullptr;

	void AudioEngine::Init()
	{
		sAudioData = new AudioEngineData();

		if (InitAL(nullptr, 0) != 0)
		{
			return;
		}

		mp3dec_init(&sAudioData->Mp3d);

		sAudioData->AudioScratchBuffer = new uint8_t[sAudioData->AudioScratchBufferSize];

		// Test audio
		LoadMP3("assets/audio/rocavaco-Bass17w19.mp3");
		LoadOgg("assets/audio/Splash.ogg");
	}

	void AudioEngine::Shutdown()
	{
		delete sAudioData;
	}

	void AudioEngine::LoadMP3(const std::filesystem::path& filename)
	{
		if (!std::filesystem::exists(filename))
		{
			HZ_CORE_LERROR("Path [{0}] is invalid", filename.string().c_str());
			return;
		}

		mp3dec_file_info_t info;
		int loadResult = mp3dec_load(&sAudioData->Mp3d, filename.string().c_str(), &info, nullptr, nullptr);
		if (loadResult < 0)
		{
			// Log Fail
			return;
		}

		uint32_t size = info.samples * sizeof(mp3d_sample_t);

		auto sampleRate = info.hz;
		auto channels = info.channels;

		ALenum alFormat = AL_FORMAT_STEREO16;
		float lenghtSeconds = size / (info.avg_bitrate_kbps * 1024.0f);

		ALuint alBuffer;
		alGenBuffers(1, &alBuffer);
		alBufferData(alBuffer, alFormat, info.buffer, size, sampleRate);

		uint32_t sourceHandle;
		alGenSources(1, &sourceHandle);
		alSourcei(sourceHandle, AL_BUFFER, alBuffer);

		if (alGetError() != AL_NO_ERROR)
		{
			// log error
			return;
		}

		alSourcePlay(sourceHandle);
	}

	void AudioEngine::LoadOgg(const std::filesystem::path& filename)
	{
		if (!std::filesystem::exists(filename))
		{
			HZ_CORE_LERROR("Path [{0}] is invalid", filename.string().c_str());
			return;
		}

		FILE* file = fopen(filename.string().c_str(), "rb");
		OggVorbis_File vorbisFile;

		int loadResult = ov_open(file, &vorbisFile, NULL, 0);
		if (loadResult < 0)
		{
			// Log error
			fclose(file);
			return;
		}

		vorbis_info* vi = ov_info(&vorbisFile, -1);
		auto sampleRate = vi->rate;
		auto channels = vi->channels;
		ALenum alFormat = AL_FORMAT_STEREO16;

		uint64_t samples = ov_pcm_total(&vorbisFile, -1);
		float lenghtSeconds = static_cast<float>(samples) / static_cast<float>(sampleRate);
		uint32_t requiredBufferSize = 2.0f * channels * samples;

		if (sAudioData->AudioScratchBufferSize < requiredBufferSize)
		{
			sAudioData->AudioScratchBufferSize = requiredBufferSize;
			delete[] sAudioData->AudioScratchBuffer;
			sAudioData->AudioScratchBuffer = new uint8_t[sAudioData->AudioScratchBufferSize];
		}

		uint8_t* oggBuffer = sAudioData->AudioScratchBuffer;
		uint8_t* bufferPtr = oggBuffer;
		
		bool hasFailed = false;
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
				hasFailed = true;
				break;
			}

			bufferPtr += lenght;
		}

		uint32_t size = bufferPtr - oggBuffer;
		ov_clear(&vorbisFile);
		fclose(file);

		if (hasFailed)
		{
			// Log Error
			return;
		}

		ALuint alBuffer;
		alGenBuffers(1, &alBuffer);
		alBufferData(alBuffer, alFormat, oggBuffer, size, sampleRate);

		uint32_t sourceHandle;
		alGenSources(1, &sourceHandle);
		alSourcei(sourceHandle, AL_BUFFER, alBuffer);

		if (alGetError() != AL_NO_ERROR)
		{
			// log error
			return;
		}

		alSourcePlay(sourceHandle);
	}
}
