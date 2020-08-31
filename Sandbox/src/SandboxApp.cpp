#include <Hazel.h>
#include <Hazel/Core/EntryPoint.h>

#include "Platform/OpenGL/OpenGLShader.h"
#include "Hazel/Events/KeyEvent.h"

#include <imgui/imgui.h>

#include <glm/gtc/matrix_transform.hpp>
#include "glm/gtc/type_ptr.hpp"

#include "Sandbox2D.h"

class ExampleLayer final : public Hazel::Layer
{
public:
	ExampleLayer()
		: Layer("Example"), _cameraController(1280.0f / 720.0f, true)
	{
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

		// -- Texture
		auto textureShader = _shaderLibrary.Load("assets/shaders/Texture.glsl");

		_coloredGridTexture = Hazel::Texture2D::Create("assets/textures/unwrap_helper.png");
		_chernoLogotexture = Hazel::Texture2D::Create("assets/textures/ChernoLogo.png");

		std::dynamic_pointer_cast<Hazel::OpenGLShader>(textureShader)->Bind();
		std::dynamic_pointer_cast<Hazel::OpenGLShader>(textureShader)->UploadUniformInt("u_Texture", 0);
		// -- Texture
	}

	void OnUpdate(Hazel::Timestep timestep) override
	{
		// Update
		_cameraController.OnUpdate(timestep);
		CalculateFPS(timestep);

		// Render
		Hazel::RenderCommand::SetClearColor(_clearColor);
		Hazel::RenderCommand::Clear();

		Hazel::Renderer::BeginScene(_cameraController.GetCamera());

		if (_shouldDrawGrid)
		{
			std::dynamic_pointer_cast<Hazel::OpenGLShader>(_flatColorShader)->Bind();
			std::dynamic_pointer_cast<Hazel::OpenGLShader>(_flatColorShader)->UploadUniformFloat4("u_Color", _squareColor);
			for (int y = 0; y < 20; y++)
			{
				for (int x = 0; x < 20; x++)
				{
					// Square
					glm::vec3 pos(x * 0.11f, y * 0.11f, 0.0f);
					auto transform = glm::translate(_identityMatrix, pos + _squarePosition) * glm::scale(_identityMatrix, glm::vec3(0.1f));
					Hazel::Renderer::Submit(_flatColorShader, _squareVertexArray, transform);
				}
			}
		}

		auto textureShader = _shaderLibrary.Get("Texture");
		if (_shouldDrawSquare)
		{
			// Texture
			_coloredGridTexture->Bind();
			Hazel::Renderer::Submit(textureShader, _squareVertexArray, glm::scale(_identityMatrix, glm::vec3(1.5f)));
		}

		if (_shouldDrawLogo)
		{
			_chernoLogotexture->Bind();
			Hazel::Renderer::Submit(textureShader, _squareVertexArray, glm::scale(_identityMatrix, glm::vec3(1.5f)));
		}

		Hazel::Renderer::EndScene();
	}

	void OnImGuiRender(Hazel::Timestep timestep) override
	{
		ImGui::Begin("Title", nullptr, ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoTitleBar);
		if (ImGui::BeginMenuBar())
		{
			if (ImGui::BeginMenu("File"))
			{
				if (ImGui::MenuItem("Exit"))
				{
					HZ_LCRITICAL("Exiting application.");
					Hazel::Application::Get().Stop();
				}
				ImGui::EndMenu();
			}
			ImGui::Text("\tFPS : %i", _currentFPS);
			ImGui::EndMenuBar();
		}
		ImGui::ColorEdit4("Back Color", glm::value_ptr(_clearColor));
		ImGui::Checkbox("Draw Grid", &_shouldDrawGrid);
		if (_shouldDrawGrid)
		{
			ImGui::ColorEdit4("Grid Color", glm::value_ptr(_squareColor));
		}
		ImGui::Checkbox("Draw Textured Square", &_shouldDrawSquare);
		ImGui::Checkbox("Draw Logo", &_shouldDrawLogo);
		ImGui::Text("Camera Control\n ASWD move\n QE rotate\n R reset\n Scroll zoom");
		auto camPos = _cameraController.GetPosition();
		ImGui::Text("Camera Info\n Rotation : %f\n Position : (x:%f, y:%f)", _cameraController.GetRotation(), camPos.x, camPos.y);
		ImGui::End();
	}

	void CalculateFPS(Hazel::Timestep timestep)
	{
		_oneSecondCountDown -= timestep;
		_frameCount++;
		if (_oneSecondCountDown <= 0.0f)
		{
			_currentFPS = _frameCount;
			_oneSecondCountDown = 1.0f;
			_frameCount = 0;
		}
	}

	void OnEvent(Hazel::Event& event) override
	{
		_cameraController.OnEvent(event);
	}

private:
	glm::vec4 _clearColor = { 0.13f, 0.0f, 0.3f, 1.0f };

	glm::mat4 _identityMatrix = glm::identity<glm::mat4>();

	glm::vec4 _squareColor = { 0.1f, 0.2f, 0.7f, 1.0f };
	glm::vec3 _squarePosition = { -1.045f, -1.045f, 0.0f };

	Hazel::ShaderLibrary _shaderLibrary;

	// Square Grid
	Hazel::Ref<Hazel::Shader> _flatColorShader;
	Hazel::Ref<Hazel::VertexArray> _squareVertexArray;

	// Textures
	Hazel::Ref<Hazel::Texture2D> _coloredGridTexture;
	Hazel::Ref<Hazel::Texture2D> _chernoLogotexture;

	Hazel::OrthographicCameraController _cameraController;

	int _frameCount = 0;
	int _currentFPS = 0;
	float _oneSecondCountDown = 1.0f;
	bool _shouldDrawLogo = true;
	bool _shouldDrawGrid = true;
	bool _shouldDrawSquare = true;
};

class Sandbox final : public Hazel::Application
{
public:
	Sandbox()
	{
		//PushLayer(new ExampleLayer());
		HZ_LINFO("There is {0} cores on this machine.", std::thread::hardware_concurrency());
		PushLayer(new Sandbox2D());
	}

	~Sandbox() = default;
};


Hazel::Application* Hazel::CreateApplication()
{
	return new Sandbox();
}
