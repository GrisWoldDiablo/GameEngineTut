#pragma once
#include "Event.h"
#include "Hazel/Core/Input.h"

namespace Hazel
{
	class MouseMovedEvent : public Event
	{
	public:
		MouseMovedEvent(float x, float y)
			: _mouseX(x), _mouseY(y)
		{
			_sDeltaX = _sPreviousMouseX - _mouseX;
			_sPreviousMouseX = _mouseX;

			_sDeltaY = _sPreviousMouseY - _mouseY;
			_sPreviousMouseY = _mouseY;
		}

		float GetX() const { return _mouseX; }
		float GetY() const { return _mouseY; }

		float GetDeltaX() const { return _sDeltaX; }
		float GetDeltaY() const { return _sDeltaY; }

		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "MouseMovedEvent : (" << _mouseX << "," << _mouseY << ")";
			ss << ", Delta(" << _sDeltaX << "," << _sDeltaY << ")";
			return ss.str();
		}

		EVENT_CLASS_TYPE(MouseMoved)
			EVENT_CLASS_CATEGORY(EventCategoryMouse | EventCategoryInput)

	private:
		inline static float _sPreviousMouseX;
		inline static float _sPreviousMouseY;

		float _sDeltaX;
		float _sDeltaY;
		float _mouseX;
		float _mouseY;
	};

	class MouseScrolledEvent : public Event
	{
	public:
		MouseScrolledEvent(float xOffset, float yOffset)
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
		MouseButtonEvent(MouseCode button)
			:_button(button)
		{}

		MouseCode _button;
	};

	class MouseButtonPressedEvent : public MouseButtonEvent
	{
	public:
		MouseButtonPressedEvent(MouseCode button)
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
		MouseButtonReleasedEvent(MouseCode button)
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
