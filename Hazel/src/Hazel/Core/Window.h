#pragma once

#include "hzpch.h"

#include "Hazel/Events/Event.h"

namespace Hazel
{
	struct WindowProps
	{
		std::string Title;
		uint32_t Width;
		uint32_t Height;
		bool VSync;

		WindowProps(const std::string& title = "Hazel Engine", uint32_t width = 1600, uint32_t height = 900, bool isVSync = true)
			: Title(title), Width(width), Height(height), VSync(isVSync) {}
	};

	// Interface representing a desktop system based Window
	class Window
	{
	public:
		using EventCallbackFn = std::function<void(Event&)>;

		virtual ~Window() = default;

		virtual void OnUpdate() = 0;
		virtual void ProcessEvents() = 0;

		virtual uint32_t GetWidth() const = 0;
		virtual uint32_t GetHeight() const = 0;

		// Window attributes
		virtual void SetEventCallback(const EventCallbackFn& callback) = 0;
		virtual void SetVSync(bool enabled) = 0;
		virtual bool IsVSync() const = 0;
		virtual void SetTitle(const std::string& title) = 0;

		virtual void* GetNativeWindow() const = 0;
		virtual std::string GetTitle() const = 0;

		// This has to be implemented per platforms. (Window/Linux/Mac)
		static Window* Create(const WindowProps& props = WindowProps());
	};
}
