#pragma once

#define  GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/string_cast.hpp"

#include <vector>

#include <spdlog/spdlog.h>
#include <spdlog/fmt/ostr.h>

namespace Hazel
{
	typedef std::function<void(const std::string&)> LogDelegate;
	
	class LoggerContainer;
	class Logger;

	class Log

	{
	public:

		static void Init();
		static std::shared_ptr<spdlog::logger>& GetCoreLogger() { return _sCoreLogger; }
		static std::shared_ptr<spdlog::logger>& GetClientLogger() { return _sClientLogger; }

		static void LogIt(const std::string& msg);
		static void Bind(const std::string& name, const Ref<LogDelegate>& logDelegate);
		static void Unbind(const Ref<LogDelegate>& logDelegate);
		static Ref<std::ostringstream> GetSS(const std::string& name);
		static Ref<Logger> GetLogger(const std::string& name);

	private:
		static Ref<LoggerContainer> _loggerContainer;
		static Ref<spdlog::logger> _sCoreLogger;
		static Ref<spdlog::logger> _sClientLogger;
	};
}

// String formatters for glm structs
template<typename OStream, glm::length_t L, typename T, glm::qualifier Q>
inline OStream& operator<<(OStream& os, const glm::vec<L, T, Q>& vector)
{
	return os << glm::to_string(vector);
}

template<typename OStream, glm::length_t C, glm::length_t R, typename T, glm::qualifier Q>
inline OStream& operator<<(OStream& os, const glm::mat<C, R, T, Q>& matrix)
{
	return os << glm::to_string(matrix);
}

template<typename OStream, typename T, glm::qualifier Q>
inline OStream& operator<<(OStream& os, const glm::qua<T, Q>& quaternion)
{
	return os << glm::to_string(quaternion);
}

// To place fields in message use curly brackets : MACRO("my field here =>{0}", field);
// Core application log Macros
#define HZ_CORE_LTRACE(...)		::Hazel::Log::GetCoreLogger()->trace(__VA_ARGS__)	 // Normal text
#define HZ_CORE_LDEBUG(...)		::Hazel::Log::GetCoreLogger()->debug(__VA_ARGS__)	 // Blue text
#define HZ_CORE_LINFO(...)		::Hazel::Log::GetCoreLogger()->info(__VA_ARGS__)	 // Green text
#define HZ_CORE_LWARN(...)		::Hazel::Log::GetCoreLogger()->warn(__VA_ARGS__)	 // Yellow text
#define HZ_CORE_LERROR(...)		::Hazel::Log::GetCoreLogger()->error(__VA_ARGS__)	 // Red text
#define HZ_CORE_LCRITICAL(...)	::Hazel::Log::GetCoreLogger()->critical(__VA_ARGS__) // White text Red background

// Normal text also called bound delegates.	
#define HZ_CORE_LLOGIT(msg)				::Hazel::Log::LogIt(msg)
#define HZ_CORE_LBIND(name, deleg)	::Hazel::Log::Bind(name, deleg)
#define HZ_CORE_LUNBIND(deleg)		::Hazel::Log::Unbind(deleg)

// Client application log Macros
#define HZ_LTRACE(...)		::Hazel::Log::GetClientLogger()->trace(__VA_ARGS__)		// Normal text
#define HZ_LDEBUG(...)		::Hazel::Log::GetClientLogger()->debug(__VA_ARGS__)		// Blue text
#define HZ_LINFO(...)		::Hazel::Log::GetClientLogger()->info(__VA_ARGS__)		// Green text
#define HZ_LWARN(...)		::Hazel::Log::GetClientLogger()->warn(__VA_ARGS__)		// Yellow text
#define HZ_LERROR(...)		::Hazel::Log::GetClientLogger()->error(__VA_ARGS__)		// Red text
#define HZ_LCRITICAL(...)	::Hazel::Log::GetClientLogger()->critical(__VA_ARGS__)	// White text Red background
