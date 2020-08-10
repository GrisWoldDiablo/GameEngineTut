#include "hzpch.h"
#include "Application.h"
#include "Events/Event.h"
#include "Hazel/Renderer/Renderer.h"
#include "Platform/Platform.h"

namespace Hazel
{
	// Static singleton access
	Application* Application::_sInstance = nullptr;

	Application::Application()
	{
		HZ_CORE_ASSERT(!_sInstance, "Application already exist!")
			// Initialize the singleton.
			_sInstance = this;

		_window = Scope<Window>(Window::Create());
		_window->SetEventCallback(HZ_BIND_EVENT_FN(OnEvent));
		
		Renderer::Init();

		// Create ImGui and push it to the layer stack as an overlay.
		_imGuiLayer = new ImGuiLayer();
		PushOverlay(_imGuiLayer);
	}

	void Application::Run()
	{
		while (_running)
		{
			auto time = Platform::GetTime();
			auto timestep = Timestep(time - _lastFrameTime);
			_lastFrameTime = time;
			
			// Go through the layers from bottom to top
			for (auto* layer : _layerStack)
			{
				layer->OnUpdate(timestep);
			}

			// Render the ImGui layer.			
			_imGuiLayer->Begin();
			for (auto* layer : _layerStack)
			{
				layer->OnImGuiRender(timestep);
			}
			_imGuiLayer->End();

			_window->OnUpdate();
		}
	}

	void Application::Stop()
	{
		_running = false;
	}

	void Application::OnEvent(Event& event)
	{
		EventDispatcher dispatcher(event);
		dispatcher.Dispatch<WindowCloseEvent>(HZ_BIND_EVENT_FN(OnWindowClose));

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
	}

	void Application::PushOverlay(Layer* overlay)
	{
		_layerStack.PushOverlay(overlay);
	}

	bool Application::OnWindowClose(WindowCloseEvent& event)
	{
		HZ_CORE_LCRITICAL("Window was closed, exiting application.");
		_running = false;
		return true;
	}
}
