#include "hzpch.h"
#include "Application.h"
#include "Hazel/Events/Event.h"
#include "Hazel/Renderer/Renderer.h"
#include "Platform/Platform.h"

namespace Hazel
{
	// Static singleton access
	Application* Application::_sInstance = nullptr;

	Application::Application(std::string name)
	{
		HZ_PROFILE_FUNCTION();

#if HZ_DEBUG
		HZ_CORE_LINFO("There is {0} cores on this machine.", std::thread::hardware_concurrency());
#endif // HZ_DEBUG

		// Initialize the singleton.
		HZ_CORE_ASSERT(!_sInstance, "Application already exist!")
		{
			_sInstance = this;
			Time::_sInstance = new Time();
		}

		_window = Scope<Window>(Window::Create(WindowProps(std::move(name))));
		_window->SetEventCallback(HZ_BIND_EVENT_FN(OnEvent));

		Renderer::Init();

		// Create ImGui and push it to the layer stack as an overlay.
		_imGuiLayer = new ImGuiLayer();
		PushOverlay(_imGuiLayer);
	}

	void Application::Run()
	{
		HZ_PROFILE_FUNCTION();

		while (_running)
		{
			HZ_PROFILE_SCOPE("Run Loop");

			auto time = Platform::GetTime();
			auto timestep = Timestep(time - _lastFrameTime);
			_lastFrameTime = time;

			Time::SetTimestep(timestep);
			Time::SetTimeElapsed(time);

			// if minimized do not bother updating
			if (!_minimized)
			{
				{
					HZ_PROFILE_SCOPE("LayerStack Update");

					// Go through the layers from bottom to top
					for (auto* layer : _layerStack)
					{
						layer->OnUpdate();
					}
				}

				_imGuiLayer->Begin();
				{
					HZ_PROFILE_SCOPE("OnImGuiRender LayerStack Update");

					// Render the ImGui layer.			
					for (auto* layer : _layerStack)
					{
						layer->OnImGuiRender();
					}
				}
				_imGuiLayer->End();
			}

			_window->OnUpdate();
		}
	}

	void Application::Stop()
	{
		HZ_PROFILE_SNAPSHOT("Application::Stop()");

		_running = false;
	}

	void Application::OnEvent(Event& event)
	{
		HZ_PROFILE_FUNCTION();

		EventDispatcher dispatcher(event);
		dispatcher.Dispatch<WindowCloseEvent>(HZ_BIND_EVENT_FN(OnWindowClose));
		dispatcher.Dispatch<WindowResizeEvent>(HZ_BIND_EVENT_FN(OnWindowResize));

		// going through the layer stack top to bottom and consume events.
		for (auto it = _layerStack.end(); it != _layerStack.begin();)
		{
			if (event.Handled)
			{
				break;
			}
			(*--it)->OnEvent(event);
		}
	}

	void Application::PushLayer(Layer* layer)
	{
		HZ_PROFILE_FUNCTION();

		_layerStack.PushLayer(layer);
		layer->OnAttach();
	}

	void Application::PushOverlay(Layer* overlay)
	{
		HZ_PROFILE_FUNCTION();

		_layerStack.PushOverlay(overlay);
		overlay->OnAttach();
	}

	bool Application::OnWindowClose(WindowCloseEvent& event)
	{
		HZ_PROFILE_FUNCTION();

		HZ_CORE_LCRITICAL("Window was closed, exiting application.");
		Stop();
		return true;
	}

	bool Application::OnWindowResize(WindowResizeEvent& event)
	{
		HZ_PROFILE_FUNCTION();

		if (event.GetWidth() == 0 || event.GetHeight() == 0)
		{
			_minimized = true;
			return true;
		}

		_minimized = false;
		Renderer::OnWindowResize(event.GetWidth(), event.GetHeight());

		return false;
	}
}
