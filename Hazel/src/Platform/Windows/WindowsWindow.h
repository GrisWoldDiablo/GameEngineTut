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

		void OnUpdate() override;

		unsigned int GetWidth() const override { return _data.Width; }
		unsigned int GetHeight() const override { return _data.Height; }
		std::string GetTitle() const override { return _data.Title; }

		//Window attributes
		void SetEventCallback(const EventCallbackFn& callback) override { _data.EventCallback = callback; }
		void SetVSync(bool enable) override;
		bool IsVSync() const override { return _data.VSync; }
		void SetTitle(const std::string& title) override;

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
			uint32_t Width;
			uint32_t Height;
			bool VSync;

			EventCallbackFn EventCallback;
		};

		WindowData _data;
	};
}
