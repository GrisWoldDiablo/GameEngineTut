#include "hzpch.h"
#include "Log.h"
#include "spdlog/sinks/stdout_color_sinks.h"

namespace Hazel
{
	// Get the static field.
	std::shared_ptr<spdlog::logger> Log::_sCoreLogger;
	std::shared_ptr<spdlog::logger> Log::_sClientLogger;

	void Log::Init()
	{
		HZ_PROFILE_FUNCTION();
		// Custom formating https://github.com/gabime/spdlog/wiki/3.-Custom-formatting
		// %^ : Start color range
		// %T : ISO 8601 time format (HH:MM:SS)
		// %n : Logger's name
		// %v : The actual text to log
		// %$ : End color range
		spdlog::set_pattern("%^[%T] %n: %v%$");

		// Make a new Core logger for the engine.
		_sCoreLogger = spdlog::stdout_color_mt("HAZEL");
		_sCoreLogger->set_level(spdlog::level::trace);

		// Make a new Client logger for the application.
		_sClientLogger = spdlog::stdout_color_mt("APP");
		_sClientLogger->set_level(spdlog::level::trace);
	}
}
