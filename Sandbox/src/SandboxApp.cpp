#include <Hazel.h>
#include "Platform/OpenGL/OpenGLShader.h"
#include "Hazel/Events/KeyEvent.h"

#include <imgui/imgui.h>

#include <glm/gtc/matrix_transform.hpp>
#include "glm/gtc/type_ptr.hpp"

class ExampleLayer final : public Hazel::Layer
{
public:
	ExampleLayer()
		: Layer("Example"), _camera(-1.6f, 1.6f, -0.9f, 0.9f)
	{
		// -- Triangle
		_triangleVertexArray = Hazel::VertexArray::Create();

		float vertices[4 * 7] =
		{
			-0.5f, -0.5f, 0.0f, 0.8f, 0.2f, 0.3f, 1.0f,
			 0.5f, -0.5f, 0.0f, 0.2f, 0.3f, 0.8f, 1.0f,
			 0.5f,  0.5f, 0.0f, 0.3f, 0.8f, 0.2f, 1.0f,
			-0.5f,  0.5f, 0.0f, 0.8f, 0.8f, 0.2f, 1.0f,
		};
		Hazel::Ref<Hazel::VertexBuffer> vertexBuffer;
		vertexBuffer = Hazel::VertexBuffer::Create(vertices, sizeof(vertices));


		Hazel::BufferLayout layout =
		{
			{Hazel::ShaderDataType::Float3, "a_Position" },
			{Hazel::ShaderDataType::Float4, "a_Color" },
		};
		vertexBuffer->SetLayout(layout);
		_triangleVertexArray->AddVertexBuffer(vertexBuffer);

		uint32_t indices[6] = { 0, 1, 2 , 2, 3, 0 };
		Hazel::Ref<Hazel::IndexBuffer> indexBuffer;
		indexBuffer = Hazel::IndexBuffer::Create(indices, sizeof(indices) / sizeof(uint32_t));
		_triangleVertexArray->SetIndexBuffer(indexBuffer);


		_shaderVertexColor = Hazel::Shader::Create("assets/shaders/VertexColor.glsl");
		// -- Triangle

		// -- Square
		_squareVertexArray = Hazel::VertexArray::Create();
		float squareVertices[4 * 5] =
		{
			-0.5f, -0.5f, 0.0f, 0.0f, 0.0f,
			 0.5f, -0.5f, 0.0f, 1.0f, 0.0f,
			 0.5f,  0.5f, 0.0f, 1.0f, 1.0f,
			-0.5f,  0.5f, 0.0f, 0.0f, 1.0f
		};
		Hazel::Ref<Hazel::VertexBuffer> squareVertexBuffer;
		squareVertexBuffer = Hazel::VertexBuffer::Create(squareVertices, sizeof(squareVertices));
		squareVertexBuffer->SetLayout(
			{
				{ Hazel::ShaderDataType::Float3, "a_Position" },
				{ Hazel::ShaderDataType::Float2, "a_TexCoord" },
			});
		_squareVertexArray->AddVertexBuffer(squareVertexBuffer);

		uint32_t squareIndices[6] = { 0, 1, 2, 2, 3, 0 };
		Hazel::Ref<Hazel::IndexBuffer> squareIndexBuffer;
		squareIndexBuffer = Hazel::IndexBuffer::Create(squareIndices, sizeof(squareIndices) / sizeof(uint32_t));
		_squareVertexArray->SetIndexBuffer(squareIndexBuffer);

		_flatColorShader = Hazel::Shader::Create("assets/shaders/FlatColor.glsl");
		// -- Square

		_textureShader = Hazel::Shader::Create("assets/shaders/Texture.glsl");

		_texture = Hazel::Texture2D::Create("assets/textures/unwrap_helper.png");
		_chernoLogotexture = Hazel::Texture2D::Create("assets/textures/ChernoLogo.png");

		std::dynamic_pointer_cast<Hazel::OpenGLShader>(_textureShader)->Bind();
		std::dynamic_pointer_cast<Hazel::OpenGLShader>(_textureShader)->UploadUniformInt("u_Texture", 0);
	}

	void OnUpdate(Hazel::Timestep timestep) override
	{
		Movement(timestep);

		Hazel::RenderCommand::SetClearColor(_clearColor);
		Hazel::RenderCommand::Clear();

		_camera.SetPosition(_cameraPosition);
		_camera.SetRotation(_cameraRotation);

		Hazel::Renderer::BeginScene(_camera);

		auto scale = glm::scale(_identityMatrix, _squareScale);

		std::dynamic_pointer_cast<Hazel::OpenGLShader>(_flatColorShader)->Bind();
		std::dynamic_pointer_cast<Hazel::OpenGLShader>(_flatColorShader)->UploadUniformFloat3("u_Color", _squareColor);
		for (int y = 0; y < 20; y++)
		{
			for (int x = 0; x < 20; x++)
			{
				// Square
				glm::vec3 pos(x * 0.11f, y * 0.11f, 0.0f);
				auto transform = glm::translate(_identityMatrix, pos + _squarePosition) * scale;
				transform *= _rotationMatrix;
				Hazel::Renderer::Submit(_flatColorShader, _squareVertexArray, transform);
			}
		}
		// Textured squares
		_texture->Bind();
		Hazel::Renderer::Submit(_textureShader, _squareVertexArray, glm::scale(_identityMatrix, glm::vec3(1.5f)));

		//Triangle
		std::dynamic_pointer_cast<Hazel::OpenGLShader>(_shaderVertexColor)->Bind();
		std::dynamic_pointer_cast<Hazel::OpenGLShader>(_shaderVertexColor)->UploadUniformFloat("u_Time", Hazel::Platform::GetTime());

		Hazel::Renderer::Submit(_shaderVertexColor, _triangleVertexArray);

		_chernoLogotexture->Bind();
		Hazel::Renderer::Submit(_textureShader, _squareVertexArray, glm::scale(_identityMatrix, glm::vec3(1.5f)));

		Hazel::Renderer::EndScene();
	}

	void Movement(Hazel::Timestep timestep)
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
			if (Hazel::Input::IsKeyPressed(HZ_KEY_LEFT_SHIFT))
			{
				_cameraRotation += _cameraRotationSpeed * timestep;
			}
			else
			{
				_squareRotation += _squareRotationSpeed * timestep;
				_rotationMatrix = glm::rotate(_identityMatrix, glm::radians(_squareRotation), glm::vec3(0, 0, 1));
			}
		}
		else if (Hazel::Input::IsKeyPressed(HZ_KEY_E))
		{
			if (Hazel::Input::IsKeyPressed(HZ_KEY_LEFT_SHIFT))
			{
				_cameraRotation -= _cameraRotationSpeed * timestep;
			}
			else
			{
				_squareRotation -= _squareRotationSpeed * timestep;
				_rotationMatrix = glm::rotate(_identityMatrix, glm::radians(_squareRotation), glm::vec3(0, 0, 1));
			}
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

	void OnImGuiRender(Hazel::Timestep timestep) override
	{
		ImGui::Begin("Select your background Color.");
		ImGui::TextColored(ImVec4(_clearColor.x, _clearColor.y, _clearColor.z, _clearColor.w), "Color");
		ImGui::ColorEdit4("Color", glm::value_ptr(_clearColor), ImGuiColorEditFlags_InputRGB);
		ImGui::ColorEdit3("Square Color", glm::value_ptr(_squareColor), ImGuiColorEditFlags_InputRGB);
		ImGui::Text("Camera Control\n Hold Left SHIFT:\n  ASWD move\n  QE rotate");
		ImGui::Text("Grid Control\n  ASWD move\n  QE rotate\n  RF scale,\n  Holding Left CTRL scale pulse");
		ImGui::Text("FPS Counter\n  Z hide/show");
		ImGui::End();

		if (_showFPS)
		{
			ShowFPS(timestep);
		}
	}

	void ShowFPS(Hazel::Timestep timestep)
	{
		// Frames per second.
		_oneSecond -= timestep;
		_frameCount++;
		if (_oneSecond <= 0.0f)
		{
			_currentFPS = _frameCount;
			_oneSecond = 1.0f;
			_frameCount = 0;
		}
		ImGui::Begin("FPS", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoFocusOnAppearing);
		ImGui::SetWindowFontScale(1.5f);
		ImGui::Text(std::to_string(_currentFPS).c_str());
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
			HZ_LCRITICAL("ESC Key pressed, exiting application.");
			break;
		case HZ_KEY_Z:
			_showFPS = !_showFPS;
			HZ_LDEBUG("Show FPS {0}", _showFPS);
			break;
		case HZ_KEY_P:
			_shaderVertexColor = Hazel::Shader::Create("assets/shaders/VertexColor.glsl");
			break;
		}
		return false;
	}

private:
	glm::vec4 _clearColor = { 0.13f, 0.0f, 0.3f, 1.0f };
	glm::vec3 _cameraPosition = { 0.0f, 0.0f, 0.0f };
	float _cameraMoveSpeed = 5.0f;
	float _cameraRotation = 0.0f;
	float _cameraRotationSpeed = 90.0f;

	glm::vec3 _squareColor = { 0.1f, 0.2f, 0.7f };

	float _squareMoveSpeed = 1.0f;
	glm::vec3 _squarePosition = { -1.045f, -1.045f, 0.0f };
	bool _squareGrow = false;
	float _scaleSpeed = 0.1f;
	glm::vec3 _squareScale = { 0.1f,0.1f,0.1f };
	float _squareRotation = 0.0f;
	float _squareRotationSpeed = 90.0f;

	glm::mat4 _identityMatrix = glm::identity<glm::mat4>();
	glm::mat4 _rotationMatrix = glm::identity<glm::mat4>();

	// Triangle
	Hazel::Ref<Hazel::Shader> _shaderVertexColor;
	Hazel::Ref<Hazel::VertexArray> _triangleVertexArray;
	//Square
	Hazel::Ref<Hazel::Shader> _flatColorShader;
	Hazel::Ref<Hazel::Shader> _textureShader;
	Hazel::Ref<Hazel::VertexArray> _squareVertexArray;

	Hazel::Ref<Hazel::Texture2D> _texture;
	Hazel::Ref<Hazel::Texture2D> _chernoLogotexture;

	Hazel::OrthographicCamera _camera;

	int _frameCount = 0;
	int _currentFPS = 0;
	float _oneSecond = 1.0f;
	bool _showFPS = true;
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
