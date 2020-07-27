#pragma once
#include "Core.h"

#include "Window.h"
#include "Hazel/LayerStack.h"
#include "Hazel/Events/ApplicationEvent.h"
#include "Hazel/Events/KeyEvent.h"

#include "Hazel/ImGui/ImGuiLayer.h"

#include "Renderer/Shader.h"
#include "Renderer/Buffer.h"
#include "Renderer/VertexArray.h"

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

	private:
		std::unique_ptr<Window> _window;
		ImGuiLayer* _imGuiLayer;
		bool _running = true;
		LayerStack _layerStack;

		std::shared_ptr<Shader> _shader;
		std::shared_ptr<VertexArray> _vertexArray;

		std::shared_ptr<Shader> _blueShader;
		std::shared_ptr<VertexArray> _squareVertexArray;

	public:
		float* ClearColor = new float[4]{ 0.13f, 0.0f, 0.3f, 1.0f }; // Purple
		float ScaleValue = 1.0f;
		float* Position = new float[2]{ 0.0f, 0.0f };

	private:
		static Application* _sInstance;
	};

	// To be define by the client.
	Application* CreateApplication();
}
