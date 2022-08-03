#pragma once

namespace Hazel
{
	typedef std::function<void(const std::string&)> LogDelegate;

	class LoggerContainer
	{
	public:
		LoggerContainer() = default;

	public:
		void LogIt(const std::string& msg);

		void Bind(const std::string& name, const Ref<LogDelegate>& logDelegate);

		void Unbind(const Ref<LogDelegate>& logDelegate);

	private:
		std::vector<Ref<LogDelegate>> _loggers;
		std::vector<std::string> _names;
	};
}
