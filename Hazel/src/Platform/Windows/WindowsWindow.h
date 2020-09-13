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
		~WindowsWindow();

		void OnUpdate() override;

		unsigned int GetWidth() const override { return _data.Width; }
		unsigned int GetHeight() const override { return _data.Height; }

		//Window attributes
		void SetEventCallback(const EventCallbackFn& callback) override { _data.EventCallback = callback; }
		void SetVSync(bool enable) override;
		bool IsVSync() const override;

		void* GetNativeWindow() const override { return _window; }

	private:
		void Init(const WindowProps& props);
		void Shutdown();

	private:
		GLFWwindow* _window;
		Scope<GraphicsContext> _context;
		
		struct WindowData
		{
		public:
			std::string Title;
			unsigned int Width;
			unsigned int Height;
			bool VSync;

			EventCallbackFn EventCallback;
		};

		WindowData _data;
	};
}
