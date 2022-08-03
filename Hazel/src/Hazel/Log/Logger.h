#pragma once

#include "Hazel/Core/Log.h"

namespace Hazel
{
	class Logger
	{
	public:
		template<typename T, typename... Args>
		void Log(const T& fmt, const Args&... args)
		{
			_spdLog->trace(fmt, args...);
		}

		template<typename T, typename... Args>
		void Error(const T& fmt, const Args&... args)
		{
			_spdLog->error(fmt, args...);
		}

		void ClearData() const { _stringStream->str(std::string()); }

		std::string GetData() const { return _stringStream->str(); }
		int GetSize() const { return static_cast<int>(_stringStream->tellp()); }

	private:
		Logger(const std::string& name, Ref<std::ostringstream>, Ref<spdlog::logger> stringStream);

		std::string _name;
		Ref<std::ostringstream> _stringStream;
		Ref<spdlog::logger> _spdLog;

		friend class Log;
	};
}
