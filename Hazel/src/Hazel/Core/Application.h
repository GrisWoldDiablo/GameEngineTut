#pragma once
#include "Core.h"

#include "Window.h"
#include "LayerStack.h"
#include "Hazel/Events/ApplicationEvent.h"

#include "Hazel/Core/Timestep.h"

#include "Hazel/ImGui/ImGuiLayer.h"

namespace Hazel
{
	/// <summary>
	/// The application class with the main run loop.
	/// </summary>
	class Application
	{
	public:
		Application();
		virtual ~Application() = default;

		void Run();
		void Stop();
		void OnEvent(Event& event);

		void PushLayer(Layer* layer);
		void PushOverlay(Layer* overlay);

		inline Window& GetWindow() { return *_window; }
		// Get the Singleton
		inline static Application& Get() { return *_sInstance; }

	private:
		bool OnWindowClose(WindowCloseEvent& event);
		bool OnWindowResize(WindowResizeEvent& event);

	private:
		Scope<Window> _window;
		ImGuiLayer* _imGuiLayer;
		LayerStack _layerStack;
		bool _running = true;
		bool _minimized = false;
		float _lastFrameTime = 0.0f;
		
	private:
		// Singleton related.
		static Application* _sInstance;
	};

	// To be define by the client.
	Application* CreateApplication();
}
