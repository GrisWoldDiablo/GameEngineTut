#pragma once

#include "Core.h"
#include "spdlog/spdlog.h"
#include "spdlog/fmt/ostr.h"

namespace Hazel
{
	class HAZEL_API Log
	{
	public:
		static void Init();
		inline static std::shared_ptr<spdlog::logger>& GetCoreLogger() { return _sCoreLogger; }
		inline static std::shared_ptr<spdlog::logger>& GetClientLogger() { return _sClientLogger; }
		   
	private:
		static std::shared_ptr<spdlog::logger> _sCoreLogger;
		static std::shared_ptr<spdlog::logger> _sClientLogger;
	};
}

// To place fields in message use curly brackets : MACRO("my field here =>{0}",field);
//Core log Macros
#define HZ_CORE_LDEBUG(...)		::Hazel::Log::GetCoreLogger()->debug(__VA_ARGS__)
#define HZ_CORE_LTRACE(...)		::Hazel::Log::GetCoreLogger()->trace(__VA_ARGS__)
#define HZ_CORE_LINFO(...)		::Hazel::Log::GetCoreLogger()->info(__VA_ARGS__)
#define HZ_CORE_LWARN(...)		::Hazel::Log::GetCoreLogger()->warn(__VA_ARGS__)
#define HZ_CORE_LERROR(...)		::Hazel::Log::GetCoreLogger()->error(__VA_ARGS__)
#define HZ_CORE_LCRITICAL(...)	::Hazel::Log::GetCoreLogger()->critical(__VA_ARGS__)

//Client log Macros
#define HZ_LDEBUG(...)		::Hazel::Log::GetClientLogger()->debug(__VA_ARGS__)
#define HZ_LTRACE(...)		::Hazel::Log::GetClientLogger()->trace(__VA_ARGS__)
#define HZ_LINFO(...)		::Hazel::Log::GetClientLogger()->info(__VA_ARGS__)
#define HZ_LWARN(...)		::Hazel::Log::GetClientLogger()->warn(__VA_ARGS__)
#define HZ_LERROR(...)		::Hazel::Log::GetClientLogger()->error(__VA_ARGS__)
#define HZ_LCRITICAL(...)	::Hazel::Log::GetClientLogger()->critical(__VA_ARGS__)
