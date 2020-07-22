#pragma once

// Macro for dll export and import

#ifdef HZ_PLATFORM_WINDOWS
	#ifdef HZ_DYNAMIC_LINK
		#ifdef HZ_BUILD_DLL
			#define HAZEL_API __declspec(dllexport)
		#else
			#define HAZEL_API __declspec(dllimport)
		#endif // HZ_BUILD_DLL  
	#else 
		#define HAZEL_API
	#endif // HZ_DYNAMIC_LINK
#else
	#error "Hazel Support only windows."
#endif // HZ_PLATFORM_WINDOWS

#ifdef HZ_DEBUG
	#ifdef HZ_PLATFORM_WINDOWS
		#define HZ_DEBUG_BREAK() __debugbreak()
	#else 
		#error "Platform does not support debugbreak!"
	#endif // HZ_PLATFORM_WINDOWS
	#define HZ_ENABLE_ASSERTS  
#endif // HZ_DEBUG

#ifdef HZ_ENABLE_ASSERTS
	#define HZ_CORE_ASSERT(x, ...) { if(!(x)) { HZ_CORE_LERROR("Assertion Failed: {0}", __VA_ARGS__); HZ_DEBUG_BREAK();} }
	#define HZ_ASSERT(x, ...) { if(!(x)) { HZ_LERROR("Assertion Failed: {0}", __VA_ARGS__); HZ_DEBUG_BREAK();} }
#else
	#define HZ_CORE_ASSERT(x, ...)
	#define HZ_ASSERT(x, ...)
#endif // HZ_ENABLE_ASSERTS

// This macro is for enum flags.
#define BIT(x) (1 << (x))

// This macro is to bind functions for callbacks, now uses lambda for faster response.
#define HZ_BIND_EVENT_FN(fn) [this](auto&&... args) -> decltype(auto) { return this->fn(std::forward<decltype(args)>(args)...); }
