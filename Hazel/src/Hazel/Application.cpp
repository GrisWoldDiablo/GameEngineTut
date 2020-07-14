#include "hzpch.h"
#include "Application.h"

#include <glad/glad.h>

namespace Hazel
{
	// Bind an event function.
#define BIND_EVENT_FN(x) std::bind(&Application::x, this, std::placeholders::_1)

	Application::Application()
	{
		m_Window = std::unique_ptr<Window>(Window::Create());
		m_Window->SetEventCallback(BIND_EVENT_FN(OnEvent));
	}

	Application::~Application()
	{
	}

	void Application::OnEvent(Event& event)
	{
		EventDispatcher dispatcher(event);
		dispatcher.Dispatch<WindowCloseEvent>(BIND_EVENT_FN(OnWindowClose));
		dispatcher.Dispatch<KeyPressedEvent>(BIND_EVENT_FN(OnKeypress));

		// going through the layerstack top to bottom and consume events.
		for (auto it = m_LayerStack.end(); it != m_LayerStack.begin();)
		{
			(*--it)->OnEvent(event);
			if (event.Handled)
			{
				break;
			}
		}

		HZ_CORE_LTRACE("{0}", event);
	}

	void Application::PushLayer(Layer* layer)
	{
		m_LayerStack.PushLayer(layer);
	}

	void Application::PushOverlay(Layer* overlay)
	{
		m_LayerStack.PopOverlay(overlay);
	}

	void Application::Run()
	{
		std::printf("\n");
		HZ_CORE_LDEBUG("Application::Run()");

		while (m_Running)
		{
			glClearColor(m_Red, m_Green, m_Blue, 1);
			glClear(GL_COLOR_BUFFER_BIT);

			// Go through the layers from bottom to top
			for (Layer* layer : m_LayerStack)
			{
				layer->OnUpdate();
			}

			m_Window->OnUpdate();
		}
	}

	bool Application::OnWindowClose(WindowCloseEvent& event)
	{
		m_Running = false;
		return true;
	}

	bool Application::OnKeypress(KeyPressedEvent& event)
	{
		
		switch (event.GetKeyCode())
		{
		case 82: // 'r'
			m_Red += 0.1f;
			if (m_Red > 1.0f)
			{
				m_Red = 0.0f;
			}
			break;
		case 71: // 'g'
			m_Green += 0.1f;
			if (m_Green > 1.0f)
			{
				m_Green = 0.0f;
			}
			break;
		case 66: // 'b'
			m_Blue += 0.1f;
			if (m_Blue > 1.0f)
			{
				m_Blue = 0.0f;
			}
			break;
		default:
			break;
		}
		HZ_CORE_LDEBUG("Color : R({0}), G({1}), B({2})", m_Red, m_Green, m_Blue);
		return true;
	}
}
