#pragma once

#include <memory>
#include "PlatformDetection.h"

#ifdef HZ_DEBUG
#	ifdef HZ_PLATFORM_WINDOWS
#		define HZ_DEBUG_BREAK() __debugbreak()
#	else 
#		error "Platform does not support debugbreak!"
#	endif // HZ_PLATFORM_WINDOWS
#	define HZ_ENABLE_ASSERTS  
#endif // HZ_DEBUG

// TODO: Make no arguments version of this macro.
#ifdef HZ_ENABLE_ASSERTS // This is a if function
#	define HZ_CORE_ASSERT(x, ...) { if(!(x)) { HZ_CORE_LERROR("Assertion Failed: {0}", __VA_ARGS__); HZ_DEBUG_BREAK();} }
#	define HZ_ASSERT(x, ...) { if(!(x)) { HZ_LERROR("Assertion Failed: {0}", __VA_ARGS__); HZ_DEBUG_BREAK();} }
#else
#	define HZ_CORE_ASSERT(x, ...)
#	define HZ_ASSERT(x, ...)
#endif // HZ_ENABLE_ASSERTS

// This macro is for enum flags.
#define BIT(x) (1 << (x))

// This macro is to bind functions for callbacks, now uses lambda for faster response.
#define HZ_BIND_EVENT_FN(fn) [this](auto&&... args) -> decltype(auto) { return this->fn(std::forward<decltype(args)>(args)...); }

// This is to join the a name with line number : use HZ_GET_LINE([name],__LINE__) to get [name##] in macros
#define HZ_JOIN_TO_LINE(name,line) name##line
#define HZ_GET_LINE(name,line) HZ_JOIN_TO_LINE(name,line)

// For creating of proper pointers
namespace Hazel
{
	template<typename T>
	using Scope = std::unique_ptr<T>;
	template<typename T, typename ... Args>
	constexpr Scope<T> CreateScope(Args&& ... args)
	{
		return std::make_unique<T>(std::forward<Args>(args)...);
	}

	template<typename T>
	using Ref = std::shared_ptr<T>;
	template<typename T, typename ... Args>
	constexpr Ref<T> CreateRef(Args&& ... args)
	{
		return std::make_shared<T>(std::forward<Args>(args)...);
	}
}
