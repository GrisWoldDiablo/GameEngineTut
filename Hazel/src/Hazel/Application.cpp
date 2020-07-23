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

		// Create ImGui and push it to the layer stack as an overlay.
		_imGuiLayer = new ImGuiLayer();
		PushOverlay(_imGuiLayer);
		ClearColor = new float[4]{ 0.13f, 0.0f, 0.3f, 1.0f }; // purple

		// -- Draw Triangle
		glGenVertexArrays(1, &_vertexArray);
		glBindVertexArray(_vertexArray);

		glGenBuffers(1, &_vertexBuffer);
		glBindBuffer(GL_ARRAY_BUFFER, _vertexBuffer);

		float vertices[3 * 3] =
		{
			-0.5f, -0.5f , 0.0f,
			 0.5f, -0.5f , 0.0f,
			 0.0f,  0.5f , 0.0f
		};

		glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), nullptr);

		glGenBuffers(1, &_indexBuffer);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _indexBuffer);

		unsigned int indices[3] = { 0, 1, 2 };
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
		// -- Draw Triangle
	}

	Application::~Application() = default;
	
	void Application::Run()
	{
		std::printf("\n");

		while (_running)
		{
			glClearColor(ClearColor[0], ClearColor[1], ClearColor[2], ClearColor[3]);
			glClear(GL_COLOR_BUFFER_BIT);

			// -- Draw Triangle
			glBindVertexArray(_vertexArray);
			glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, nullptr);
			// -- Draw Triangle

			// Go through the layers from bottom to top
			for (auto* layer : _layerStack)
			{
				layer->OnUpdate();
			}

			// Render the ImGui layer.			
			_imGuiLayer->Begin();
			for (auto* layer : _layerStack)
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
	}

	void Application::PushOverlay(Layer* overlay)
	{
		_layerStack.PushOverlay(overlay);
	}

	bool Application::OnWindowClose(WindowCloseEvent& event)
	{
		_running = false;
		return true;
	}

	bool Application::OnKeypress(KeyPressedEvent& event)
	{
		switch (event.GetKeyCode())
		{
		case 256: // ESC
			_running = false;
			HZ_CORE_LCRITICAL("ESC Key pressed, exiting application.");
			break;
		default:
			break;
		}

		return true;
	}
}
