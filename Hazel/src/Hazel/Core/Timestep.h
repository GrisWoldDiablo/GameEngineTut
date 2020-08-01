#pragma once

namespace Hazel
{
	class Timestep
	{
	public:
		Timestep(float time = 0.0f)
			:_time(time)
		{
		}

		// When casting to float it returns _time
		operator float() const { return _time; }
		
		float GetSeconds() const { return _time; }
		float GetMilliseconds() const { return _time * 1000.0f; }

	private:
		float _time;
	};
}
