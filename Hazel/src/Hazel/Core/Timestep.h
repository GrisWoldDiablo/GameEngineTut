#pragma once

namespace Hazel
{
	class Timestep
	{
	public:
		Timestep(float time = 0.0f)
			:_time(time)
		{}

		// When casting to float it returns _time
		operator float() const { return _time; }

		float GetSeconds() const { return _time; }
		float GetMilliseconds() const { return _time * 1000.0f; }

	private:
		float _time;
	};

	class Time
	{
	private:
		Time() = default;
		Timestep _timestep;

	public:
		static Timestep GetTimestep() { return _sInstance->_timestep; }
		static float GetTimeElapsed() { return _sInstance->_sTimeElapsed; }

	private:

		static void SetTimestep(Timestep timestep) { _sInstance->_timestep = timestep; }
		static void SetTimeElapsed(float time) { _sTimeElapsed = time; }

	private:
		inline static Time* _sInstance;
		inline static float _sTimeElapsed;

		friend class Application;
	};
}
