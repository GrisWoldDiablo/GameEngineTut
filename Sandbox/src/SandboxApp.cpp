#include <Hazel.h>
#include <imgui/imgui.h>


#include <glm/gtc/matrix_transform.hpp>

#include "Hazel/Events/KeyEvent.h"

class ExampleLayer final : public Hazel::Layer
{
public:
	ExampleLayer()
		: Layer("Example"), _camera(-1.6f, 1.6f, -0.9f, 0.9f)
	{
		// -- Triangle
		_triangleVertexArray.reset(Hazel::VertexArray::Create());

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
		_triangleVertexArray->AddVertexBuffer(vertexBuffer);

		uint32_t indices[3] = { 0, 1, 2 };
		std::shared_ptr<Hazel::IndexBuffer> indexBuffer;
		indexBuffer.reset(Hazel::IndexBuffer::Create(indices, sizeof(indices) / sizeof(uint32_t)));
		_triangleVertexArray->SetIndexBuffer(indexBuffer);

		std::string vertexSrc = R"(
			#version 430
			layout(location = 0) in vec3 a_Position;
			layout(location = 1) in vec4 a_Color;

			uniform mat4 u_ViewProjection;
			uniform mat4 u_Transform;
		
			out vec3 v_Position;
			out vec4 v_Color;

			void main()
			{
				v_Color = a_Color;
				v_Position = a_Position;
				gl_Position = u_ViewProjection * u_Transform * vec4(v_Position, 1.0);
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
			-0.5f, -0.5f, 0.0f,
			 0.5f, -0.5f, 0.0f,
			 0.5f,  0.5f, 0.0f,
			-0.5f,  0.5f, 0.0f
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

		std::string flatColorShaderVertexSrc = R"(
			#version 430
			layout(location = 0) in vec3 a_Position;

			uniform mat4 u_ViewProjection;
			uniform mat4 u_Transform;
;

			out vec3 v_Position;

			void main()
			{
				v_Position = a_Position;
				gl_Position = u_ViewProjection * u_Transform * vec4(v_Position, 1.0);
			}
		)";

		std::string flatColorShaderFragmentShaderSrc = R"(
			#version 430
			layout(location = 0) out vec4 color;

			in vec3 v_Position;
			uniform vec4 u_Color;
		
			void main()
			{
				color = u_Color;
			}
		)";

		_flatColorShader.reset(new Hazel::Shader(flatColorShaderVertexSrc, flatColorShaderFragmentShaderSrc));
		// -- Square
	}

	void OnUpdate(Hazel::Timestep timestep) override
	{
		Movement(timestep);

		Hazel::RenderCommand::SetClearColor({ _clearColor[0], _clearColor[1], _clearColor[2], _clearColor[3] });
		Hazel::RenderCommand::Clear();

		Hazel::Renderer::BeginScene(_camera);

		_camera.SetPosition(_cameraPosition);
		_camera.SetRotation(_cameraRotation);
		// Triangle
		Hazel::Renderer::Submit(_shader, _triangleVertexArray);

		auto scale = glm::scale(glm::identity<glm::mat4>(), _squareScale);
		glm::vec4 redColor(0.7f, 0.1f, 0.2f, 1.0f);
		glm::vec4 blueColor(0.1f, 0.2f, 0.7f, 1.0f);

		//Hazel::MaterialRef material = new Hazel::Material(_flatColorShader);
		//Hazel::MaterialInstanceRef materialInstance = new Hazel::MaterialInstance(material);
		//
		//materialInstance->Set("u_Color", redColor);
		//squareMesh->SetMaterial(materialInstance);
		
		for (int y = 0; y < 20; y++)
		{
			for (int x = 0; x < 20; x++)
			{
				// Square
				glm::vec3 pos(x * 0.11f, y * 0.11f, 0.0f);
				auto transform = glm::translate(glm::identity<glm::mat4>(), pos + _squarePosition) * scale;
				if ((x+y) % 2 == 0)
				{
					_flatColorShader->UploadUniformFloat4("u_Color", redColor);
				}
				else
				{

					_flatColorShader->UploadUniformFloat4("u_Color", blueColor);
				}
				Hazel::Renderer::Submit(_flatColorShader, _squareVertexArray, transform);
			}
		}

		Hazel::Renderer::EndScene();
	}

	void Movement(float timestep)
	{
		if (Hazel::Input::IsKeyPressed(HZ_KEY_W))
		{
			if (Hazel::Input::IsKeyPressed(HZ_KEY_LEFT_SHIFT))
			{
				_cameraPosition.y += _cameraMoveSpeed * timestep;
			}
			else
			{
				_squarePosition.y += _squareMoveSpeed * timestep;
			}
		}
		else if (Hazel::Input::IsKeyPressed(HZ_KEY_S))
		{
			if (Hazel::Input::IsKeyPressed(HZ_KEY_LEFT_SHIFT))
			{
				_cameraPosition.y -= _cameraMoveSpeed * timestep;
			}
			else
			{
				_squarePosition.y -= _squareMoveSpeed * timestep;
			}
		}

		if (Hazel::Input::IsKeyPressed(HZ_KEY_A))
		{
			if (Hazel::Input::IsKeyPressed(HZ_KEY_LEFT_SHIFT))
			{
				_cameraPosition.x -= _cameraMoveSpeed * timestep;
			}
			else
			{
				_squarePosition.x -= _squareMoveSpeed * timestep;
			}
		}
		else if (Hazel::Input::IsKeyPressed(HZ_KEY_D))
		{
			if (Hazel::Input::IsKeyPressed(HZ_KEY_LEFT_SHIFT))
			{
				_cameraPosition.x += _cameraMoveSpeed * timestep;
			}
			else
			{
				_squarePosition.x += _squareMoveSpeed * timestep;
			}
		}

		if (Hazel::Input::IsKeyPressed(HZ_KEY_Q))
		{
			_cameraRotation += _cameraRotationSpeed * timestep;
		}
		else if (Hazel::Input::IsKeyPressed(HZ_KEY_E))
		{
			_cameraRotation -= _cameraRotationSpeed * timestep;
		}


		if (Hazel::Input::IsKeyPressed(HZ_KEY_LEFT_CONTROL))
		{
			if (_squareGrow)
			{
				_squareScale += _scaleSpeed * timestep;
				if (glm::greaterThan(_squareScale, glm::vec3(0.107f)).x)
				{
					_squareGrow = false;
				}
			}
			else
			{
				_squareScale -= _scaleSpeed * timestep;
				if (glm::lessThanEqual(_squareScale, glm::vec3(0.01f)).x)
				{
					_squareGrow = true;
				}
			}
		}
		else
		{
			if (Hazel::Input::IsKeyPressed(HZ_KEY_R))
			{
				_squareScale += _scaleSpeed * timestep;
			}
			else if (Hazel::Input::IsKeyPressed(HZ_KEY_F))
			{
				_squareScale -= _scaleSpeed * timestep;
			}
		}
		_squareScale = glm::clamp(_squareScale, (0.01f), (0.107f));
	}

	void OnImGuiRender() override
	{
		ImGui::Begin("Select your background Color.");
		ImGui::TextColored(ImVec4(_clearColor[0], _clearColor[1], _clearColor[2], _clearColor[3]), "Color");
		ImGui::ColorEdit4("Color", _clearColor, ImGuiColorEditFlags_InputRGB);
		ImGui::Text("Camera Control\n Hold Left SHIFT:\n  ASWD move\n  QE rotate");
		ImGui::Text("Grid Control\n  ASWD move\n  QE rotate\n  RF scale,\n  Holding Left CTRL scale pulse");
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
	float _cameraMoveSpeed = 5.0f;
	float _cameraRotation = 0.0f;
	float _cameraRotationSpeed = 90.0f;

	float _squareMoveSpeed = 1.0f;
	glm::vec3 _squarePosition = { -1.045f, -1.045f, 0.0f };
	bool _squareGrow = false;
	float _scaleSpeed = 0.1f;
	glm::vec3 _squareScale = { 0.1f,0.1f,0.1f };

	// Triangle
	std::shared_ptr<Hazel::Shader> _shader;
	std::shared_ptr<Hazel::VertexArray> _triangleVertexArray;
	//Square
	std::shared_ptr<Hazel::Shader> _flatColorShader;
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
