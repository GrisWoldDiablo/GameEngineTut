#pragma once
#include "Event.h"
#include "Hazel/Core/MouseCodes.h"

namespace Hazel
{
	class MouseMovedEvent : public Event
	{
	public:
		MouseMovedEvent(const float x, const float y)
			: _mouseX(x), _mouseY(y)
		{
			_deltaX = _sPreviousMouseX - _mouseX;
			_sPreviousMouseX = _mouseX;

			_deltaY = _sPreviousMouseY - _mouseY;
			_sPreviousMouseY = _mouseY;
		}

		float GetX() const { return _mouseX; }
		float GetY() const { return _mouseY; }

		float GetDeltaX() const { return _deltaX; }
		float GetDeltaY() const { return _deltaY; }

		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "MouseMovedEvent : (" << _mouseX << "," << _mouseY << ")";
			ss << ", Delta(" << _deltaX << "," << _deltaY << ")";
			return ss.str();
		}

		EVENT_CLASS_TYPE(MouseMoved)
			EVENT_CLASS_CATEGORY(EventCategoryMouse | EventCategoryInput)

	private:
		inline static float _sPreviousMouseX;
		inline static float _sPreviousMouseY;

		float _deltaX;
		float _deltaY;
		float _mouseX;
		float _mouseY;
	};

	class MouseScrolledEvent : public Event
	{
	public:
		MouseScrolledEvent(const float xOffset, const float yOffset)
			: _xOffset(xOffset), _yOffset(yOffset)
		{}

		float GetXOffset() const { return _xOffset; }
		float GetYOffset() const { return _yOffset; }

		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "MouseScrolledEvent : (" << _xOffset << "," << _yOffset << ")";
			return ss.str();
		}

		EVENT_CLASS_TYPE(MouseScrolled)
			EVENT_CLASS_CATEGORY(EventCategoryMouse | EventCategoryInput)

	private:
		float _xOffset;
		float _yOffset;
	};

	class MouseButtonEvent : public Event
	{
	public:
		MouseCode GetMouseButton() const { return _button; }

		EVENT_CLASS_CATEGORY(EventCategoryMouse | EventCategoryInput | EventCategoryMouseButton)

	protected:
		MouseButtonEvent(const MouseCode button)
			:_button(button)
		{}

		MouseCode _button;
	};

	class MouseButtonPressedEvent : public MouseButtonEvent
	{
	public:
		MouseButtonPressedEvent(const MouseCode button)
			: MouseButtonEvent(button)
		{}

		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "MouseButtonPressedEvent : " << _button;
			return ss.str();
		}

		EVENT_CLASS_TYPE(MouseButtonPressed)
	};

	class MouseButtonReleasedEvent : public MouseButtonEvent
	{
	public:
		MouseButtonReleasedEvent(const MouseCode button)
			: MouseButtonEvent(button)
		{}

		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "MouseButtonReleasedEvent : " << _button;
			return ss.str();
		}

		EVENT_CLASS_TYPE(MouseButtonReleased)
	};
}
