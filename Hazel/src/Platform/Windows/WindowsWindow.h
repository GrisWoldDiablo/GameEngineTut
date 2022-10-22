#pragma once

#include "Hazel/Core/Window.h"

#include <GLFW/glfw3.h>

#include "Hazel/Renderer/GraphicsContext.h"

namespace Hazel
{
	class WindowsWindow final : public Window
	{
	public:
		WindowsWindow(const WindowProps& props);
		virtual ~WindowsWindow() override;

		virtual void OnUpdate() override;
		virtual void ProcessEvents() override;

		virtual unsigned int GetWidth() const override { return _data.Width; }
		virtual unsigned int GetHeight() const override { return _data.Height; }
		virtual std::string GetTitle() const override { return _data.Title; }

		//Window attributes
		virtual void SetEventCallback(const EventCallbackFn& callback) override { _data.EventCallback = callback; }
		virtual void SetVSync(bool enable) override;
		virtual bool IsVSync() const override { return _data.VSync; }
		virtual void SetTitle(const std::string& title) override;

		virtual void* GetNativeWindow() const override { return _window; }

	private:
		void Init(const WindowProps& props);
		void Shutdown();

	private:
		GLFWwindow* _window;
		Scope<GraphicsContext> _context;

		struct WindowData
		{
			std::string Title;
			uint32_t Width;
			uint32_t Height;
			bool VSync;

			EventCallbackFn EventCallback;
		};

		WindowData _data;
	};
}
