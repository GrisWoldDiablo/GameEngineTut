#pragma once
#include "hzpch.h"
#include "Hazel/Core/Base.h"

namespace Hazel
{
	// Events in Hazel are currently blocking.

	/// <summary>
	/// The type of the event.
	/// </summary>
	enum class EventType
	{
		None = 0,
		WindowClose, WindowResize, WindowFocus, WindowLostFocus, WindowMoved,
		AppTick, AppUpdate, AppRender,
		KeyPressed, KeyReleased, KeyTyped,
		MouseButtonPressed, MouseButtonReleased, MouseMoved, MouseScrolled
	};

	/// <summary>
	/// The category the event is part of.
	/// </summary>
	enum EventCategory
	{
		None = 0,
		EventCategoryApplication = BIT(0),
		EventCategoryInput = BIT(1),
		EventCategoryKeyboard = BIT(2),
		EventCategoryMouse = BIT(3),
		EventCategoryMouseButton = BIT(4)
	};

	// This macro is to simplify the definition of event classes.
#define EVENT_CLASS_TYPE(type) static EventType GetStaticType() { return EventType::type;}\
								virtual EventType GetEventType() const override { return GetStaticType(); }\
								virtual const char* GetName() const override { return #type; }

#define EVENT_CLASS_CATEGORY(category) virtual int GetCategoryFlags() const override { return category; }

	/// <summary>
	/// Base event class
	/// </summary>
	class Event
	{
	private:
		friend class EventDispatcher;

	public:
		virtual ~Event() = default;

		bool Handled = false;

		virtual EventType GetEventType() const = 0;
		virtual const char* GetName() const = 0;
		virtual int GetCategoryFlags() const = 0;
		virtual std::string ToString() const { return GetName(); }

		bool IsInCategory(EventCategory category)
		{
			return GetCategoryFlags() & category;
		}
	};

	/// <summary>
	/// The class that will send the event to be processed.
	/// </summary>
	class EventDispatcher
	{
		template<typename T>
		using EventFn = std::function<bool(T&)>;
	public:
		virtual ~EventDispatcher() = default;

		EventDispatcher(Event& event)
			: _event(event)
		{}

		template<typename T>
		bool Dispatch(EventFn<T> func)
		{
			if (_event.GetEventType() == T::GetStaticType())
			{
				_event.Handled = func(*(T*)&_event);
				return true;
			}
			return false;
		}

	private:
		Event& _event;
	};

	inline std::ostream& operator<<(std::ostream& os, const Event& event)
	{
		return os << event.ToString();
	}
}
