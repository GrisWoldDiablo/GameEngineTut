#include "hzpch.h"
#include "Log.h"
#include "LoggerContainer.h"
#include "Hazel/Log/Logger.h"

#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/sinks/ostream_sink.h"

namespace Hazel
{
	// Get the static field.
	Ref<LoggerContainer> Log::_loggerContainer;
	Ref<spdlog::logger> Log::_sCoreLogger;
	Ref<spdlog::logger> Log::_sClientLogger;

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

		_loggerContainer = CreateRef<LoggerContainer>();
	}

	void Log::LogIt(const std::string& msg)
	{
		_loggerContainer->LogIt(msg);
	}

	void Log::Bind(const std::string& name, const Ref<LogDelegate>& logDelegate)
	{
		_loggerContainer->Bind(name, logDelegate);
	}

	void Log::Unbind(const Ref<LogDelegate>& logDelegate)
	{
		_loggerContainer->Unbind(logDelegate);
	}

	Ref<std::ostringstream> Log::GetSS(const std::string& name)
	{
		auto oss = CreateRef<std::ostringstream>();
		auto sink = CreateRef<spdlog::sinks::ostream_sink_mt>(*oss.get());
		sink->set_pattern("[%T] [%^%n%$]: %v");
		sink->set_level(spdlog::level::trace);

		_sCoreLogger->sinks().push_back(sink);

		return oss;
	}

	Ref<Logger> Log::GetLogger(const std::string& name)
	{
		auto oss = CreateRef<std::ostringstream>();
		auto ostream_sink = CreateRef<spdlog::sinks::ostream_sink_mt>(*oss.get());
		ostream_sink->set_pattern("[%T] [%n:%l]: %v");
		ostream_sink->set_level(spdlog::level::trace);
		auto spdLogger = CreateRef<spdlog::logger>(name, ostream_sink);
		spdLogger->set_level(spdlog::level::trace);
		_sCoreLogger->sinks().push_back(ostream_sink);

		Ref<Logger> logger(new Logger(name, oss, spdLogger));
		//logger.reset(new Logger(name, spdLogger));
		return logger;
	}
}
