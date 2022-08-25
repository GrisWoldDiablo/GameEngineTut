#pragma once

#include "AudioTypes.h"

namespace Hazel
{
	class AudioSource
	{
	public:
		static Ref<AudioSource> Create(const std::filesystem::path& path);

		AudioSource() = default;
		AudioSource(uint32_t alBuffer, const std::filesystem::path& path, float length, AudioFileFormat fileFormat);
		~AudioSource();

		void Play();
		void Stop();
		void Pause();
		void Rewind();

		void SetGain(float gain);
		void SetPitch(float pitch);
		void Set3D(bool is3D);
		void SetLoop(bool isLoop);
		void SetPosition(const glm::vec3& position);

		float GetGain() const { return _gain; }
		float GetPitch() const { return _pitch; }
		bool Get3D() const { return _is3D; }
		bool GetLoop() const { return _isLoop; }
		const glm::vec3& GetPosition() const { return _position; }

		AudioSourceState GetState();
		float GetOffset();
		void SetOffset(float offset);
		const std::filesystem::path& GetPath() const { return _path; }
		float GetLength() const { return _length; }


	private:
		uint32_t _alBuffer = 0;
		uint32_t _alSource = 0;
		std::filesystem::path _path;
		float _length;
		AudioFileFormat _fileFormat;


		float _gain = 1.0f;
		float _pitch = 1.0f;
		bool _is3D = false;
		bool _isLoop = false;
		glm::vec3 _position{ 0.0f, 0.0f, 0.0f };

		friend class AudioEngine;
	};
}
