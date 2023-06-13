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
#ifdef HZ_ENABLE_ASSERTS // This is an (if not) function
#	define HZ_CORE_ASSERT(x, ...) { if(!(x)) { HZ_CORE_LERROR("Assertion Failed: {0}", __VA_ARGS__); HZ_DEBUG_BREAK();} } void(0)
#	define HZ_CORE_ASSERT_ONCE(x, ...) { static bool hasAsserted = false; if(!hasAsserted) { hasAsserted = true; HZ_CORE_ASSERT(x, __VA_ARGS__); } } void(0)
#	define HZ_ASSERT(x, ...) { if(!(x)) { HZ_LERROR("Assertion Failed: {0}", __VA_ARGS__); HZ_DEBUG_BREAK();} } void(0)
#	define HZ_ASSERT_ONCE(x, ...) { static bool hasAsserted = false; if(!hasAsserted) { hasAsserted = true; HZ_ASSERT(x, __VA_ARGS__);} } void(0)

// Ensure can be used as conditions if (HZ_CORE_ENSURE(true)) { // execute logic }
#	define HZ_CORE_ENSURE(x) ((x) || ([] { HZ_DEBUG_BREAK(); } (), false))
#	define HZ_CORE_ENSURE_ONCE(x) ((x) || ([] { static bool hasEnsured = false; if (!hasEnsured) { hasEnsured = true; HZ_DEBUG_BREAK(); } } (), false))
#	define HZ_CORE_ENSURE_MSG(x, ...) ((x) || ([] { HZ_CORE_LERROR("Ensure Failed: {0}", __VA_ARGS__); HZ_DEBUG_BREAK(); } (), false))
#	define HZ_ENSURE(x) HZ_CORE_ENSURE(x)
#	define HZ_ENSURE_ONCE(x) HZ_CORE_ENSURE_ONCE(x)
#	define HZ_ENSURE_MSG(x, ...) ((x) || ([] { HZ_LERROR("Ensure Failed: {0}", __VA_ARGS__); HZ_DEBUG_BREAK(); } (), false))
#else
#	define HZ_CORE_ASSERT(x, ...)
#	define HZ_CORE_ASSERT_ONCE(x, ...)
#	define HZ_ASSERT(x, ...)
#	define HZ_ASSERT_ONCE(x, ...)

#	define HZ_CORE_ENSURE(x) ((x))
#	define HZ_CORE_ENSURE_ONCE(x) ((x))
#	define HZ_CORE_ENSURE_MSG(x, ...) ((x))
#	define HZ_ENSURE(x) ((x))
#	define HZ_ENSURE_ONCE(x) ((x))
#	define HZ_ENSURE_MSG(x, ...) ((x))
#endif // HZ_ENABLE_ASSERTS

// This macro is for enum flags.
#define BIT(x) (1 << (x))

// This macro is to bind functions for callbacks, now uses lambda for faster response.
#define HZ_BIND_EVENT_FN(fn) [this](auto&&... args) -> decltype(auto) { return this->fn(std::forward<decltype(args)>(args)...); }

// This is to join the a name with line number : use HZ_GET_LINE([name],__LINE__) to get [name##] in macros
#define HZ_JOIN_TO_LINE(name, line) name##line
#define HZ_GET_LINE(name, line) HZ_JOIN_TO_LINE(name, line)

// For creating of proper pointers
namespace Hazel
{
	template<typename T>
	using Scope = std::unique_ptr<T>;

	template<typename T, typename... Args>
	constexpr Scope<T> CreateScope(Args&&... args)
	{
		return std::make_unique<T>(std::forward<Args>(args)...);
	}

	template<typename T>
	using Ref = std::shared_ptr<T>;

	template<typename T, typename... Args>
	constexpr Ref<T> CreateRef(Args&&... args)
	{
		return std::make_shared<T>(std::forward<Args>(args)...);
	}

	template<typename T>
	using Weak = std::weak_ptr<T>;
}
