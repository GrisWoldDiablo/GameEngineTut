#pragma once
#include "Core.h"

#include "Window.h"
#include "Hazel/LayerStack.h"
#include "Hazel/Events/ApplicationEvent.h"
#include "Hazel/Events/KeyEvent.h"

namespace Hazel
{
	/// <summary>
	/// The application class with the main run loop.
	/// </summary>
	class HAZEL_API Application
	{
	public:
		Application();
		virtual ~Application();

		void Run();

		void OnEvent(Event& event);

		void PushLayer(Layer* layer);
		void PushOverlay(Layer* overlay);

		inline static Application& Get() { return *s_Instance; }
		inline Window& GetWindow() { return *m_Window; }

	private:
		bool OnWindowClose(WindowCloseEvent& event);
		bool OnKeypress(KeyPressedEvent& event);

		std::unique_ptr<Window> m_Window;
		bool m_Running = true;
		LayerStack m_LayerStack;

		float m_Red = 0.0f, m_Green = 0.0f, m_Blue = 0.0f;

	private:
		static Application* s_Instance;
	};

	// To be define by the client.
	Application* CreateApplication();
}
