#pragma  once

#include <chrono>

namespace Hazel
{
	class Timer
	{
	public:
		Timer()
		{

			Reset();
		}


		void Timer::Reset()
		{
			_start = std::chrono::high_resolution_clock::now();
		}

		float Timer::Elapsed()
		{
			return std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now() - _start).count() * 0.001f * 0.001f * 0.001f;
		}

		float Timer::ElapsedMillis()
		{
			return Elapsed() * 1000.0f;
		}

	private:
		std::chrono::time_point<std::chrono::high_resolution_clock> _start;
	};
}
