#pragma once

#include <utility>


#include "hzpch.h"

#include "Hazel/Core/Core.h"
#include "Hazel/Events/Event.h"

namespace Hazel
{
	struct WindowProps
	{
	public:
		std::string Title;
		unsigned int Width;
		unsigned int Height;

		WindowProps(std::string title = "Hazel Engine",
		            unsigned int width = 1280,
		            unsigned int height = 720)
			: Title(std::move(title)), Width(width), Height(height) {}
	};

	// Interface representing a desktop system based Window
	class Window
	{
	public:
		using EventCallbackFn = std::function<void(Event&)>;

		virtual ~Window() = default;

		virtual void OnUpdate() = 0;

		virtual unsigned int GetWidth() const = 0;
		virtual unsigned int GetHeight() const = 0;

		// Window attributes
		virtual void SetEventCallback(const EventCallbackFn& callback) = 0;
		virtual void SetVSync(bool enabled) = 0;
		virtual bool IsVSync() const = 0;

		virtual void* GetNativeWindow() const = 0;

		// This has to be implemented per platforms. (window/linux/mac)
		static Window* Create(const WindowProps& props = WindowProps());
	};
}
