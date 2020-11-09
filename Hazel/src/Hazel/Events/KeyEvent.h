#pragma once
#include "Event.h"
#include "Hazel/Core/Input.h"

namespace Hazel
{
	class KeyEvent : public Event
	{
	public:
		KeyCode GetKeyCode() const { return _keyCode; }

		EVENT_CLASS_CATEGORY(EventCategoryKeyboard | EventCategoryInput)

	protected:
		KeyEvent(KeyCode key) 
			: _keyCode(key) {}

		KeyCode _keyCode;
	};

	class KeyPressedEvent : public KeyEvent
	{
	public:
		KeyPressedEvent(KeyCode key, int repeatCount)
			: KeyEvent(key), _repeatCount(repeatCount) {}

		int GetRepeatCount() const { return _repeatCount; }

		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "KeyPressedEvent : " << _keyCode << " (" << _repeatCount << " repeats)";
			return ss.str();
		}

		EVENT_CLASS_TYPE(KeyPressed)

	private:
		int _repeatCount;
	};

	class KeyReleasedEvent : public KeyEvent
	{
	public:
		KeyReleasedEvent(KeyCode keyCode)
			: KeyEvent(keyCode) {}

		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "KeyReleasedEvent : " << _keyCode;
			return ss.str();
		}

		EVENT_CLASS_TYPE(KeyReleased)
	};

	class KeyTypedEvent : public KeyEvent
	{
	public:
		KeyTypedEvent(KeyCode key)
			: KeyEvent(key)	{}

		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "KeyTypedEvent : " << _keyCode;
			return ss.str();
		}

		EVENT_CLASS_TYPE(KeyTyped)
	};
}
