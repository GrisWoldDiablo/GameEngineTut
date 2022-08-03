#include "hzpch.h"
#include "Logger.h"

namespace Hazel
{
	Logger::Logger(const std::string& name, Ref<std::ostringstream> stringStream, Ref<spdlog::logger> spdLog)
		:_name(name), _stringStream(stringStream), _spdLog(spdLog)
	{
	}
}
