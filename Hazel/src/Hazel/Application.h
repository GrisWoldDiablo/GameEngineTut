#pragma once
#include "Core.h"

#include "Window.h"
#include "Hazel/LayerStack.h"
#include "Hazel/Events/ApplicationEvent.h"
#include "Hazel/Events/KeyEvent.h"

#include "Hazel/ImGui/ImGuiLayer.h"

namespace Hazel
{
	/// <summary>
	/// The application class with the main run loop.
	/// </summary>
	class HAZEL_API Application
	{
	public:
		Application();
		virtual ~Application() = default;

		void Run();

		void OnEvent(Event& event);

		void PushLayer(Layer* layer);
		void PushOverlay(Layer* overlay);

		inline static Application& Get() { return *_sInstance; }
		inline Window& GetWindow() { return *_window; }

	private:
		bool OnWindowClose(WindowCloseEvent& event);
		bool OnKeypress(KeyPressedEvent& event);

		std::unique_ptr<Window> _window;
		ImGuiLayer* _imGuiLayer;
		bool _running = true;
		LayerStack _layerStack;

		float _red = 0.0f, _green = 0.0f, _blue = 0.0f;

	private:
		static Application* _sInstance;
	};

	// To be define by the client.
	Application* CreateApplication();
}
