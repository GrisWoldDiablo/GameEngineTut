#pragma once

enum class AudioFileFormat : uint8_t
{
	NONE = 0,
	MP3,
	OGG
};

enum class AudioSourceState : int
{
	NONE = 0,
	INITIAL = 0x1011,
	PLAYING = 0x1012,
	PAUSED = 0x1013,
	STOPPED = 0x1014
};
