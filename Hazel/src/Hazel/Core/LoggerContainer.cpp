#include "hzpch.h"
#include "Log.h"
#include "LoggerContainer.h"

namespace Hazel
{
	void LoggerContainer::LogIt(const std::string& msg)
	{
		std::stringstream ss;
		for (size_t i = 0; i < _loggers.size(); i++)
		{
			ss << "[" << _names[i] << "]: " << msg;
			HZ_CORE_LTRACE(msg);
			(*_loggers[i])(msg);
			ss.str(std::string());
		}
	}

	void LoggerContainer::Bind(const std::string& name, const Ref<LogDelegate>& logDelegate)
	{
		_loggers.push_back(logDelegate);
		_names.push_back(name);
	}

	void LoggerContainer::Unbind(const Ref<LogDelegate>& logDelegate)
	{
		for (int i = 0; i < _loggers.size(); i++)
		{
			if (_loggers[i] = logDelegate)
			{
				_loggers.erase(_loggers.begin() + i);
				_names.erase(_names.begin() + i);
				break;
			}
		}
	}
}
