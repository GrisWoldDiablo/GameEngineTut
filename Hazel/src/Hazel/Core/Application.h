#pragma once
#include "Base.h"

#include "Window.h"
#include "LayerStack.h"
#include "Hazel/Events/ApplicationEvent.h"
#include "Hazel/ImGui/ImGuiLayer.h"

namespace Hazel
{
	int Main(int argc, char** argv);

	struct ApplicationCommandLineArgs
	{
		int Count = 0;
		char** Args = nullptr;

		const char* operator[](int index) const
		{
			HZ_CORE_ASSERT(index < Count, "Index out of range");
			return Args[index];
		}
	};

	struct ApplicationSpecification
	{
		std::string Name = "Hazel Application";
		std::string WorkingDirectory;
		ApplicationCommandLineArgs CommandLineArgs;
	};

	/// <summary>
	/// The application class with the main run loop.
	/// </summary>
	class Application
	{
	public:
		Application(const ApplicationSpecification& specification);
		virtual ~Application();

		void Stop();
		void ReloadScriptEngine();

		void OnEvent(Event& event);
		void PushLayer(Layer* layer);
		void PushOverlay(Layer* overlay);

		ImGuiLayer* GetImGuiLayer() { return _imGuiLayer; }

		Window& GetWindow() { return *_window; }
		// Get the Singleton
		static Application& Get() { return *_sInstance; }

		const ApplicationSpecification& GetSpecification() const { return _specification; }

		void SubmitToMainThread(const std::function<void()>& function);

	private:
		void Run();
		bool OnWindowClose(const WindowCloseEvent& windowCloseEvent);
		bool OnWindowResize(const WindowResizeEvent& windowResizeEvent);
		void ExecuteMainThreadQueue();

	private:
		ApplicationSpecification _specification;
		Scope<Window> _window;
		ImGuiLayer* _imGuiLayer;
		LayerStack _layerStack;
		bool _running = true;
		bool _minimized = false;
		float _lastFrameTime = 0.0f;

		std::vector<std::function<void()>> _mainThreadQueue;
		std::mutex _mainThreadQueueMutex;

	private:
		// Singleton related.
		static Application* _sInstance;
		friend int Hazel::Main(int argc, char** argv);
	};

	// To be define by the client.
	Application* CreateApplication(ApplicationCommandLineArgs args);
}
