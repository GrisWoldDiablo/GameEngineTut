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

#include "Hazel/Renderer/OrthographicCamera.h"

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

	public:
		float* ClearColor = new float[4]{ 0.13f, 0.0f, 0.3f, 1.0f }; // Purple
		glm::vec3 CameraPosition = { 0.0f,0.0f,0.0f };
		float CameraRotation = 0.0f;
	private:
		std::unique_ptr<Window> _window;
		ImGuiLayer* _imGuiLayer;
		LayerStack _layerStack;
		bool _running = true;

		// Triangle
		std::shared_ptr<Shader> _shader;
		std::shared_ptr<VertexArray> _vertexArray;
		//Square
		std::shared_ptr<Shader> _blueShader;
		std::shared_ptr<VertexArray> _squareVertexArray;

		OrthographicCamera _camera;

	private:
		static Application* _sInstance;
	};

	// To be define by the client.
	Application* CreateApplication();
}
