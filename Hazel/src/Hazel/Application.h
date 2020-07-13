#pragma once
#include "Core.h"
#include "Hazel/Events/ApplicationEvent.h"

#include "Window.h"

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
		void OnEvent(Event& e);

	private:
		bool OnWindowClose(WindowCloseEvent& e);

		std::unique_ptr<Window> m_Window;
		bool m_Running = true;
	};

	// To be define by the client.
	Application* CreateApplication();
}
