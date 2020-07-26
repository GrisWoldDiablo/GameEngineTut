#include "hzpch.h"
#include "Application.h"

#include <glad/glad.h>

#include "GLFW/glfw3.h"

namespace Hazel
{
	// Static singleton access
	Application* Application::_sInstance = nullptr;

	static GLenum ShaderDataTypeToOpenGLBaseType(ShaderDataType type)
	{
		switch (type)
		{
			case ShaderDataType::Float:	return GL_FLOAT;
			case ShaderDataType::Float2: return GL_FLOAT;
			case ShaderDataType::Float3: return GL_FLOAT;
			case ShaderDataType::Float4: return GL_FLOAT;
			case ShaderDataType::Mat3:	return GL_FLOAT;
			case ShaderDataType::Mat4:	return GL_FLOAT;
			case ShaderDataType::Int:	return GL_INT;
			case ShaderDataType::Int2:	return GL_INT;
			case ShaderDataType::Int3:	return GL_INT;
			case ShaderDataType::Int4:	return GL_INT;
			case ShaderDataType::Bool:	return GL_BOOL;
		}

		HZ_CORE_ASSERT(false, "Unknown ShaderDataType!");
		return 0;
	}

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
		// -- Draw Triangle
		glGenVertexArrays(1, &_vertexArray);
		glBindVertexArray(_vertexArray);

		float vertices[6 * 7] =
		{
			-0.5f, -0.5f, 0.0f, 0.8f, 0.2f, 0.8f, 1.0f,
			 0.0f, -0.5f, 0.0f, 0.2f, 0.3f, 0.8f, 1.0f,
			-0.25f, 0.5f,-0.0f, 0.8f, 0.8f, 0.2f, 1.0f,
			 0.5f, -0.5f, 0.0f, 0.2f, 0.3f, 0.8f, 1.0f,
			 0.25f, 0.5f, 0.0f, 0.8f, 0.8f, 0.2f, 1.0f,
			 0.0f, -1.0f, 0.0f, 0.8f, 0.0f, 0.0f, 1.0f,
		};
		_vertexBuffer.reset(VertexBuffer::Create(vertices, sizeof(vertices)));
		
		{
			BufferLayout layout =
			{
				{ ShaderDataType::Float3, "a_Position" },
				{ ShaderDataType::Float4, "a_Color" },
			};
			_vertexBuffer->SetLayout(layout);
		}

		uint32_t index = 0;
		const auto& layout = _vertexBuffer->GetLayout();
		for (const auto& element : layout)
		{
			glEnableVertexAttribArray(index);
			glVertexAttribPointer(index,
				element.GetComponentCount(),
				_vertexBuffer->GetBaseType(element),
				element.Normalized,
				layout.GetStride(),
				(const void*)element.Offset);
			index++;
		}

		uint32_t indices[9] = { 0, 1, 2, 0, 5, 3, 1, 3, 4 };
		_indexBuffer.reset(IndexBuffer::Create(indices, sizeof(indices) / sizeof(uint32_t)));
		// -- Draw Triangle

		std::string vertexSrc = R"(
			#version 430
			layout(location = 0) in vec3 a_Position;
			layout(location = 1) in vec4 a_Color;

			uniform float _time;
			uniform float _scale;
			uniform vec2 _position;
			out vec3 v_Position;
			out vec4 v_Color;

			void main()
			{
				v_Color = a_Color;
				v_Position = a_Position;
				v_Position.y += sin(_time) * 0.1;
				v_Position *= _scale;
				//v_Position.y *= _scale;
				v_Position.x += cos(_time) * 0.1;
				v_Position.xy += _position.xy;
				gl_Position = vec4(v_Position, 1.0);
			}
		)";

		std::string fragmentSrc = R"(
			#version 430
			layout(location = 0) out vec4 color;
			uniform vec4 _color;

			uniform float _time;
			in vec3 v_Position;
			in vec4 v_Color;
			
			void main()
			{
				color = v_Color;
				color *= _color;
				//color += vec4(v_Position * 0.5 + 0.5, 1.0);
				//color.x += sin(_time)*2.0;
			}
		)";

		_shader.reset(new Shader(vertexSrc, fragmentSrc));
		
	}

	void Application::Run()
	{
		auto timeLoc = glGetUniformLocation(_shader->GetRendererID(), "_time");
		auto colorLoc = glGetUniformLocation(_shader->GetRendererID(), "_color");
		auto scaleLoc = glGetUniformLocation(_shader->GetRendererID(), "_scale");
		auto positionLoc = glGetUniformLocation(_shader->GetRendererID(), "_position");
		std::printf("\n");
		while (_running)
		{
			glClearColor(ClearColor[0], ClearColor[1], ClearColor[2], ClearColor[3]);
			glClear(GL_COLOR_BUFFER_BIT);

			auto time = (float)glfwGetTime();
			// -- Draw Triangle
			glUniform1f(timeLoc, time);
			glUniform4f(colorLoc, ClearColor[0], ClearColor[1], ClearColor[2], ClearColor[3]);
			glUniform1f(scaleLoc, ScaleValue);
			glUniform2f(positionLoc, Position[0],Position[1]);

			_shader->Bind();
			glBindVertexArray(_vertexArray);
			glDrawElements(GL_TRIANGLES, _indexBuffer->GetCount(), GL_UNSIGNED_INT, nullptr);
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
