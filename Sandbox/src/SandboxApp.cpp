#include <Hazel.h>
#include <imgui/imgui.h>

#include "Hazel/Events/KeyEvent.h"

class ExampleLayer final : public Hazel::Layer
{
public:
	ExampleLayer()
		: Layer("Example"), _camera(-1.6f, 1.6f, -0.9f, 0.9f)
	{
		// -- Triangle
		_vertexArray.reset(Hazel::VertexArray::Create());

		float vertices[3 * 7] =
		{
			-0.5f, -0.5f, 0.0f, 0.8f, 0.2f, 0.8f, 1.0f,
			 0.5f, -0.5f, 0.0f, 0.2f, 0.3f, 0.8f, 1.0f,
			 0.0f,  0.5f, 0.0f, 0.8f, 0.8f, 0.2f, 1.0f,
		};
		std::shared_ptr<Hazel::VertexBuffer> vertexBuffer;
		vertexBuffer.reset(Hazel::VertexBuffer::Create(vertices, sizeof(vertices)));


		Hazel::BufferLayout layout =
		{
			{Hazel::ShaderDataType::Float3, "a_Position" },
			{Hazel::ShaderDataType::Float4, "a_Color" },
		};
		vertexBuffer->SetLayout(layout);
		_vertexArray->AddVertexBuffer(vertexBuffer);

		uint32_t indices[3] = { 0, 1, 2 };
		std::shared_ptr<Hazel::IndexBuffer> indexBuffer;
		indexBuffer.reset(Hazel::IndexBuffer::Create(indices, sizeof(indices) / sizeof(uint32_t)));
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

		_shader.reset(new Hazel::Shader(vertexSrc, fragmentSrc));
		// -- Triangle

		// -- Square
		_squareVertexArray.reset(Hazel::VertexArray::Create());
		float squareVertices[4 * 3] =
		{
			-0.75f, -0.75f, 0.0f,
			 0.75f, -0.75f, 0.0f,
			 0.75f,  0.75f, 0.0f,
			-0.75f,  0.75f, 0.0f
		};
		std::shared_ptr<Hazel::VertexBuffer> squareVertexBuffer;
		squareVertexBuffer.reset(Hazel::VertexBuffer::Create(squareVertices, sizeof(squareVertices)));
		squareVertexBuffer->SetLayout(
			{
				{ Hazel::ShaderDataType::Float3, "a_Position" },
			});
		_squareVertexArray->AddVertexBuffer(squareVertexBuffer);

		uint32_t squareIndices[6] = { 0, 1, 3, 3, 1, 2 };
		std::shared_ptr<Hazel::IndexBuffer> squareIndexBuffer;
		squareIndexBuffer.reset(Hazel::IndexBuffer::Create(squareIndices, sizeof(squareIndices) / sizeof(uint32_t)));
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

		_blueShader.reset(new Hazel::Shader(blueShaderVertexSrc, blueFragmentShaderSrc));
		// -- Square
	}

	void OnUpdate(Hazel::Timestep timestep) override
	{
		CameraMovement(timestep);
		
		Hazel::RenderCommand::SetClearColor({ _clearColor[0], _clearColor[1], _clearColor[2], _clearColor[3] });
		Hazel::RenderCommand::Clear();

		Hazel::Renderer::BeginScene(_camera);

		_camera.SetPosition(_cameraPosition);
		_camera.SetRotation(_cameraRotation);

		// Triangle
		Hazel::Renderer::Submit(_blueShader, _squareVertexArray);
		// Square
		Hazel::Renderer::Submit(_shader, _vertexArray);

		Hazel::Renderer::EndScene();
	}

	void CameraMovement(float timestep)
	{
		if (Hazel::Input::IsKeyPressed(HZ_KEY_W))
		{
			_cameraPosition.y += _movementSpeed * timestep;
		}
		else if (Hazel::Input::IsKeyPressed(HZ_KEY_S))
		{
			_cameraPosition.y -= _movementSpeed * timestep;
		}
		
		if (Hazel::Input::IsKeyPressed(HZ_KEY_A))
		{
			_cameraPosition.x -= _movementSpeed * timestep;
		}
		else if (Hazel::Input::IsKeyPressed(HZ_KEY_D))
		{
			_cameraPosition.x += _movementSpeed * timestep;
		}
		
		if (Hazel::Input::IsKeyPressed(HZ_KEY_Q))
		{
			_cameraRotation += _rotationSpeed * timestep;
		}
		else if (Hazel::Input::IsKeyPressed(HZ_KEY_E))
		{
			_cameraRotation -= _rotationSpeed * timestep;
		}
	}

	void OnImGuiRender() override
	{
		ImGui::Begin("Select your background Color.");
		ImGui::TextColored(ImVec4(_clearColor[0], _clearColor[1], _clearColor[2], _clearColor[3]), "Color");
		ImGui::ColorEdit4("Color", _clearColor, ImGuiColorEditFlags_InputRGB);
		ImGui::End();
	}

	void OnEvent(Hazel::Event& event) override
	{
		Hazel::EventDispatcher dispatcher(event);
		dispatcher.Dispatch<Hazel::KeyPressedEvent>(HZ_BIND_EVENT_FN(OnKeyPressedEvent));
	}

	bool OnKeyPressedEvent(Hazel::KeyPressedEvent& event)
	{
		switch (event.GetKeyCode())
		{
		case HZ_KEY_ESCAPE:
			Hazel::Application::Get().Stop();
			HZ_CORE_LCRITICAL("ESC Key pressed, exiting application.");
			break;
		}
		return false;
	}

private:
	float* _clearColor = new float[4]{ 0.13f, 0.0f, 0.3f, 1.0f };
	glm::vec3 _cameraPosition = { 0.0f,0.0f,0.0f };
	float _movementSpeed = 5.0f;
	float _cameraRotation = 0.0f;
	float _rotationSpeed = 90.0f;

	// Triangle
	std::shared_ptr<Hazel::Shader> _shader;
	std::shared_ptr<Hazel::VertexArray> _vertexArray;
	//Square
	std::shared_ptr<Hazel::Shader> _blueShader;
	std::shared_ptr<Hazel::VertexArray> _squareVertexArray;

	Hazel::OrthographicCamera _camera;
};

class Sandbox final : public Hazel::Application
{
public:
	Sandbox()
	{
		PushLayer(new ExampleLayer());
	}

	~Sandbox() = default;
};


Hazel::Application* Hazel::CreateApplication()
{
	return new Sandbox();
}
