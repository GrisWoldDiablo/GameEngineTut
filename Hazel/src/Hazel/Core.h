#pragma once

// Macro for dll export and import

#ifdef HZ_PLATFORM_WINDOWS
	#ifdef HZ_BUILD_DLL
		#define HAZEL_API __declspec(dllexport)
	#else
		#define HAZEL_API __declspec(dllimport)
	#endif // HZ_BUILD_DLL
#else
	#error "Hazel Support only windows.""
#endif // HZ_PLATFORM_WINDOWS

#ifdef HZ_DEBUG
	#ifdef HZ_PLATFORM_WINDOWS
		#define HZ_DEBUGBREAK() __debugbreak()
	#else 
		#error "Platform does not support debugbreak!"
	#endif // HZ_PLATFORM_WINDOWS
	#define HZ_ENABLE_ASSERTS  
#endif // HZ_DEBUG

#ifdef HZ_ENABLE_ASSERTS
	#define HZ_CORE_ASSERT(x, ...) { if(!(x)) { HZ_CORE_LERROR("Assertion Failed: {0}", __VA_ARGS__); HZ_DEBUGBREAK();} }
	#define HZ_ASSERT(x, ...) { if(!(x)) { HZ_LERROR("Assertion Failed: {0}", __VA_ARGS__); HZ_DEBUGBREAK();} }
#else
	#define HZ_CORE_ASSERT(x, ...)
	#define HZ_ASSERT(x, ...)
#endif // HZ_ENABLE_ASSERTS

// This macro is for enum flags.
#define BIT(x) (1 << x)

// This macro is to bind functions for callbacks.
#define HZ_BIND_EVENT_FN(fn) std::bind(&fn, this, std::placeholders::_1)
