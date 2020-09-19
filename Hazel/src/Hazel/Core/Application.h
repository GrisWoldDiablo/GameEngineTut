#pragma once
#include "Base.h"

#include "Window.h"
#include "LayerStack.h"
#include "Hazel/Events/ApplicationEvent.h"

#include "Hazel/Core/Timestep.h"

#include "Hazel/ImGui/ImGuiLayer.h"

int main(int argc, char** argv);

namespace Hazel
{
	/// <summary>
	/// The application class with the main run loop.
	/// </summary>
	class Application
	{
	public:
		Application();
		virtual ~Application();

		void Stop();
		void OnEvent(Event& event);

		void PushLayer(Layer* layer);
		void PushOverlay(Layer* overlay);

		Window& GetWindow() { return *_window; }
		// Get the Singleton
		static Application& Get() { return *_sInstance; }

	private:
		void Run();
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
		friend int ::main(int argc, char** argv);
	};

	// To be define by the client.
	Application* CreateApplication();
}
