#pragma once
#include "Event.h"
#include "Hazel/Core/KeyCodes.h"

namespace Hazel
{
	class KeyEvent : public Event
	{
	public:
		KeyCode GetKeyCode() const { return _keyCode; }

		EVENT_CLASS_CATEGORY(EventCategoryKeyboard | EventCategoryInput)

	protected:
		KeyEvent(const KeyCode keyCode)
			: _keyCode(keyCode)
		{}

		KeyCode _keyCode;
	};

	class KeyPressedEvent : public KeyEvent
	{
	public:
		KeyPressedEvent(const KeyCode keyCode, const bool isRepeat = false)
			: KeyEvent(keyCode), _isRepeat(isRepeat)
		{}

		bool IsRepeat() const { return _isRepeat; }

		std::string ToString() const override
		{
			return fmt::format("KeyPressedEvent : {0} (repeat={1})", _keyCode, _isRepeat);
		}

		EVENT_CLASS_TYPE(KeyPressed)

	private:
		bool _isRepeat;
	};

	class KeyReleasedEvent : public KeyEvent
	{
	public:
		KeyReleasedEvent(const KeyCode keyCode)
			: KeyEvent(keyCode)
		{}

		std::string ToString() const override
		{
			return fmt::format("KeyReleasedEvent : {0}", _keyCode);
		}

		EVENT_CLASS_TYPE(KeyReleased)
	};

	class KeyTypedEvent : public KeyEvent
	{
	public:
		KeyTypedEvent(const KeyCode keyCode)
			: KeyEvent(keyCode)
		{}

		std::string ToString() const override
		{
			return fmt::format("KeyTypedEvent : {0}", _keyCode);
		}

		EVENT_CLASS_TYPE(KeyTyped)
	};
}
