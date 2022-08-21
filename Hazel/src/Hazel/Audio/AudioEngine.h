#pragma once

namespace Hazel
{
	class AudioEngine
	{
	public:
		static void Init();
		static void Shutdown();

	private:
		static void LoadMP3(const std::filesystem::path& filename);
		static void LoadOgg(const std::filesystem::path& filename);

	};
}
