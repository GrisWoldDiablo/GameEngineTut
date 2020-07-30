#include "hzpch.h"
#include "Application.h"

#include "Hazel/Renderer/Renderer.h"
#include <glm/gtc/random.hpp>

namespace Hazel
{
	// Static singleton access
	Application* Application::_sInstance = nullptr;

	Application::Application()
		:_camera(-1.6f, 1.6f, -0.9f, 0.9f)
	{
		HZ_CORE_ASSERT(!_sInstance, "Application already exist!")
			// Initialize the singleton.
			_sInstance = this;

		_window = std::unique_ptr<Window>(Window::Create());
		_window->SetEventCallback(HZ_BIND_EVENT_FN(OnEvent));

		// Create ImGui and push it to the layer stack as an overlay.
		_imGuiLayer = new ImGuiLayer();
		PushOverlay(_imGuiLayer);

		// -- Triangle
		_vertexArray.reset(VertexArray::Create());

		float vertices[3 * 7] =
		{
			-0.5f, -0.5f, 0.0f, 0.8f, 0.2f, 0.8f, 1.0f,
			 0.5f, -0.5f, 0.0f, 0.2f, 0.3f, 0.8f, 1.0f,
			 0.0f,  0.5f, 0.0f, 0.8f, 0.8f, 0.2f, 1.0f,
		};
		std::shared_ptr<VertexBuffer> vertexBuffer;
		vertexBuffer.reset(VertexBuffer::Create(vertices, sizeof(vertices)));


		BufferLayout layout =
		{
			{ ShaderDataType::Float3, "a_Position" },
			{ ShaderDataType::Float4, "a_Color" },
		};
		vertexBuffer->SetLayout(layout);
		_vertexArray->AddVertexBuffer(vertexBuffer);

		uint32_t indices[3] = { 0, 1, 2 };
		std::shared_ptr<IndexBuffer> indexBuffer;
		indexBuffer.reset(IndexBuffer::Create(indices, sizeof(indices) / sizeof(uint32_t)));
		_vertexArray->SetIndexBuffer(indexBuffer);

		std::string vertexSrc = R"(
			#version 430
			layout(location = 0) in vec3 a_Position;
			layout(location = 1) in vec4 a_Color;

			uniform mat4 u_ViewProjection;
		
			out vec3 v_Position;
			out vec4 v_Color;

			void main()
			{
				v_Color = a_Color;
				v_Position = a_Position;
				gl_Position = u_ViewProjection * vec4(v_Position, 1.0);
			}
		)";

		std::string fragmentSrc = R"(
			#version 430
			layout(location = 0) out vec4 color;

			in vec3 v_Position;
			in vec4 v_Color;
			
			void main()
			{
				color = v_Color;
			}
		)";

		_shader.reset(new Shader(vertexSrc, fragmentSrc));
		// -- Triangle

		// -- Square
		_squareVertexArray.reset(VertexArray::Create());
		float squareVertices[4 * 3] =
		{
			-0.75f, -0.75f, 0.0f,
			 0.75f, -0.75f, 0.0f,
			 0.75f,  0.75f, 0.0f,
			-0.75f,  0.75f, 0.0f
		};
		std::shared_ptr<VertexBuffer> squareVertexBuffer;
		squareVertexBuffer.reset(VertexBuffer::Create(squareVertices, sizeof(squareVertices)));
		squareVertexBuffer->SetLayout(
			{
				{ ShaderDataType::Float3, "a_Position" },
			});
		_squareVertexArray->AddVertexBuffer(squareVertexBuffer);

		uint32_t squareIndices[6] = { 0, 1, 3, 3, 1, 2 };
		std::shared_ptr<IndexBuffer> squareIndexBuffer;
		squareIndexBuffer.reset(IndexBuffer::Create(squareIndices, sizeof(squareIndices) / sizeof(uint32_t)));
		_squareVertexArray->SetIndexBuffer(squareIndexBuffer);

		std::string blueShaderVertexSrc = R"(
			#version 430
			layout(location = 0) in vec3 a_Position;

			uniform mat4 u_ViewProjection;

			out vec3 v_Position;

			void main()
			{
				v_Position = a_Position;
				gl_Position = u_ViewProjection * vec4(v_Position, 1.0);
			}
		)";

		std::string blueFragmentShaderSrc = R"(
			#version 430
			layout(location = 0) out vec4 color;

			in vec3 v_Position;
			
			void main()
			{
				color = vec4(0.1, 0.2, 0.7, 1.0);
			}
		)";

		_blueShader.reset(new Shader(blueShaderVertexSrc, blueFragmentShaderSrc));
		// -- Square
	}

	void Application::Run()
	{
		while (_running)
		{
			RenderCommand::SetClearColor({ ClearColor[0], ClearColor[1], ClearColor[2], ClearColor[3] });
			RenderCommand::Clear();

			Renderer::BeginScene(_camera);

			_camera.SetPosition({ 0.5f,0.5f,0.0f });
			_camera.SetRotation(45.0f);
			// Triangle
			Renderer::Submit(_blueShader, _squareVertexArray);
			// Square
			Renderer::Submit(_shader, _vertexArray);

			Renderer::EndScene();

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
		HZ_CORE_LCRITICAL("Window was closed, exiting application.");
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

		return false;
	}
}
