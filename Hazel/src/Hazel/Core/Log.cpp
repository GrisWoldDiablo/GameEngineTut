#include "hzpch.h"
#include "Log.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/sinks/basic_file_sink.h"

namespace Hazel
{
	// Get the static field.
	Ref<spdlog::logger> Log::_sCoreLogger;
	Ref<spdlog::logger> Log::_sClientLogger;

	void Log::Init()
	{
		HZ_PROFILE_FUNCTION();

		const std::filesystem::path target = "Hazel.log";

		if (std::filesystem::exists(target))
		{
			const std::filesystem::path backupTarget = target.stem().string().append("_Backup").append(target.extension().string());
			std::filesystem::copy_file(target, backupTarget, std::filesystem::copy_options::overwrite_existing);
		}

		std::vector<spdlog::sink_ptr> logSinks;
		logSinks.emplace_back(CreateRef<spdlog::sinks::stdout_color_sink_mt>());
		logSinks.emplace_back(CreateRef<spdlog::sinks::basic_file_sink_mt>(target.string(), true));

		// Custom formatting https://github.com/gabime/spdlog/wiki/3.-Custom-formatting
		// %^ : Start color range
		// %T : ISO 8601 time format (HH:MM:SS)
		// %n : Logger's name
		// %v : The actual text to log
		// %$ : End color range
		// %l : The log level of the message
		logSinks[0]->set_pattern("%^[%T] %n: %v%$");
		logSinks[1]->set_pattern("[%T] [%l] %n: %v");

		// Make a new Core logger for the engine.
		_sCoreLogger = CreateRef<spdlog::logger>("HAZEL", logSinks.begin(), logSinks.end());
		spdlog::register_logger(_sCoreLogger);
		_sCoreLogger->set_level(spdlog::level::trace);
		_sCoreLogger->flush_on(spdlog::level::trace);

		// Make a new Client logger for the application.
		_sClientLogger = CreateRef<spdlog::logger>("APP", logSinks.begin(), logSinks.end());
		spdlog::register_logger(_sClientLogger);
		_sClientLogger->set_level(spdlog::level::trace);
		_sClientLogger->flush_on(spdlog::level::trace);
	}
}
