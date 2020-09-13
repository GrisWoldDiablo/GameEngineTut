#pragma once
#include "Event.h"

namespace Hazel
{
	class WindowResizeEvent : public Event
	{
	public:
		WindowResizeEvent(unsigned int width, unsigned int height)
			: _width(width), _height(height){}

		unsigned int GetWidth() const { return _width; }
		unsigned int GetHeight() const { return _height; }

		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "WindowResizeEvent : " << _width << " x " << _height;
			return ss.str();
		}

		EVENT_CLASS_TYPE(WindowResize)
		EVENT_CLASS_CATEGORY(EventCategoryApplication)
	
	private:
		unsigned int _width;
		unsigned int _height;
	};

	class WindowCloseEvent : public Event
	{
	public:
		WindowCloseEvent() = default;

		EVENT_CLASS_TYPE(WindowClose)
		EVENT_CLASS_CATEGORY(EventCategoryApplication)
	};

	class WindowMovedEvent : public Event
	{
	public:
		WindowMovedEvent(int x, int y)
			: _windowX(x), _windowY(y) {}

		unsigned int GetX() const { return _windowX; }
		unsigned int GetY() const { return _windowY; }

		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "WindowMovedEvent : (" << _windowX << "," << _windowY << ")";
			return ss.str();
		}

		EVENT_CLASS_TYPE(WindowMoved)
		EVENT_CLASS_CATEGORY(EventCategoryApplication)

	private:
		unsigned int _windowX, _windowY;
	};

	class AppTickEvent : public Event
	{
	public:
		AppTickEvent() = default;

		EVENT_CLASS_TYPE(AppTick)
		EVENT_CLASS_CATEGORY(EventCategoryApplication)

	};

	class AppUpdateEvent : public Event
	{
	public:
		AppUpdateEvent() = default;

		EVENT_CLASS_TYPE(AppUpdate)
		EVENT_CLASS_CATEGORY(EventCategoryApplication)

	};

	class AppRenderEvent : public Event
	{
	public:
		AppRenderEvent() = default;

		EVENT_CLASS_TYPE(AppRender)
		EVENT_CLASS_CATEGORY(EventCategoryApplication)

	};
}
