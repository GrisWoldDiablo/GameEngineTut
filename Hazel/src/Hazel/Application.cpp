#include "hzpch.h"
#include "Application.h"

#include <glad/glad.h>

namespace Hazel
{
	// Static singleton access
	Application* Application::_sInstance = nullptr;

	Application::Application()
	{
		HZ_CORE_ASSERT(!_sInstance, "Application already exist!")
		// Initialize the singleton.
		_sInstance = this;

		_window = std::unique_ptr<Window>(Window::Create());
		_window->SetEventCallback(HZ_BIND_EVENT_FN(OnEvent));

		_imGuiLayer = new ImGuiLayer();
		PushOverlay(_imGuiLayer);
	}
	
	void Application::Run()
	{
		std::printf("\n");
		HZ_CORE_LINFO("OpenGL Info:");
		HZ_CORE_LINFO("  Vendor: {0}", glGetString(GL_VENDOR));
		HZ_CORE_LINFO("  Renderer: {0}", glGetString(GL_RENDERER));
		HZ_CORE_LINFO("  Version: {0}", glGetString(GL_VERSION));

		while (_running)
		{
			glClearColor(_red, _green, _blue, 1);
			glClear(GL_COLOR_BUFFER_BIT);

			// Go through the layers from bottom to top
			for (Layer* layer : _layerStack)
			{
				layer->OnUpdate();
			}

			// Render the ImGui layer.			
			_imGuiLayer->Begin();
			for (Layer* layer : _layerStack)
			{
				layer->OnImGuiRender();
			}
			_imGuiLayer->End();
			
			_window->OnUpdate();
		}
	}

	void Application::OnEvent(Event& event)
	{
		EventDispatcher dispatcher(event);
		dispatcher.Dispatch<WindowCloseEvent>(HZ_BIND_EVENT_FN(OnWindowClose));
		dispatcher.Dispatch<KeyPressedEvent>(HZ_BIND_EVENT_FN(OnKeypress));

		// going through the layerstack top to bottom and consume events.
		for (auto it = _layerStack.end(); it != _layerStack.begin();)
		{
			(*--it)->OnEvent(event);
			if (event.Handled)
			{
				break;
			}
		}
	}

	void Application::PushLayer(Layer* layer)
	{
		_layerStack.PushLayer(layer);
		layer->OnAttach();
	}

	void Application::PushOverlay(Layer* overlay)
	{
		_layerStack.PushOverlay(overlay);
		overlay->OnAttach();
	}

	
	bool Application::OnWindowClose(WindowCloseEvent& event)
	{
		_running = false;
		return true;
	}

	bool Application::OnKeypress(KeyPressedEvent& event)
	{
		bool colorChanged = false;
		switch (event.GetKeyCode())
		{
		case 82: // 'r'
			_red += 0.1f;
			if (_red > 1.0f)
			{
				_red = 0.0f;
			}
			colorChanged = true;
			break;
		case 71: // 'g'
			_green += 0.1f;
			if (_green > 1.0f)
			{
				_green = 0.0f;
			}
			colorChanged = true;
			break;
		case 66: // 'b'
			_blue += 0.1f;
			if (_blue > 1.0f)
			{
				_blue = 0.0f;
			}
			colorChanged = true;
			break;
		case 256: // ESC
			_running = false;
			HZ_CORE_LWARN("ESC Key pressed, exiting application.");
			break;
		default:
			break;
		}

		if (colorChanged)
		{
			HZ_CORE_LDEBUG("Color : R({0}), G({1}), B({2})", _red, _green, _blue);
		}

		return true;
	}
}
