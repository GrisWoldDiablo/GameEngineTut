#include "Sandbox2D.h"

#include <imgui/imgui.h>

#include <glm/gtc/type_ptr.hpp>

#include "Platform/OpenGL/OpenGLShader.h"

Sandbox2D::Sandbox2D()
	: Layer("Sandbox 2D"), _cameraController(1280.0f / 720.0f, true)
{
}

void Sandbox2D::OnAttach()
{
	// -- Square
	_squareVertexArray = Hazel::VertexArray::Create();

	float squareVertices[4 * 3] =
	{
		-0.5f, -0.5f, 0.0f,
		 0.5f, -0.5f, 0.0f,
		 0.5f,  0.5f, 0.0f,
		-0.5f,  0.5f, 0.0f
	};

	Hazel::Ref<Hazel::VertexBuffer> squareVertexBuffer;
	squareVertexBuffer = Hazel::VertexBuffer::Create(squareVertices, sizeof(squareVertices));
	squareVertexBuffer->SetLayout(
		{
			{ Hazel::ShaderDataType::Float3, "a_Position" },
		});
	_squareVertexArray->AddVertexBuffer(squareVertexBuffer);

	uint32_t squareIndices[6] = { 0, 1, 2, 2, 3, 0 };
	Hazel::Ref<Hazel::IndexBuffer> squareIndexBuffer;
	squareIndexBuffer = Hazel::IndexBuffer::Create(squareIndices, sizeof(squareIndices) / sizeof(uint32_t));
	_squareVertexArray->SetIndexBuffer(squareIndexBuffer);

	_flatColorShader = Hazel::Shader::Create("assets/shaders/FlatColor.glsl");
	// -- Square
}

void Sandbox2D::OnDetach()
{

}

void Sandbox2D::OnUpdate(Hazel::Timestep timestep)
{
	// Update
	_cameraController.OnUpdate(timestep);
	CalculateFPS(timestep);

	// Render
	Hazel::RenderCommand::SetClearColor(_clearColor);
	Hazel::RenderCommand::Clear();

	Hazel::Renderer::BeginScene(_cameraController.GetCamera());

	std::dynamic_pointer_cast<Hazel::OpenGLShader>(_flatColorShader)->Bind();
	std::dynamic_pointer_cast<Hazel::OpenGLShader>(_flatColorShader)->UploadUniformFloat4("u_Color", _squareColor);
	Hazel::Renderer::Submit(_flatColorShader, _squareVertexArray);

	Hazel::Renderer::EndScene();
}

void Sandbox2D::OnImGuiRender(Hazel::Timestep timestep)
{
	ImGui::Begin("Settings", nullptr, ImGuiWindowFlags_MenuBar);
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
	ImGui::ColorEdit4("Square Color", glm::value_ptr(_squareColor));
	ImGui::End();
}

void Sandbox2D::OnEvent(Hazel::Event& event)
{
	_cameraController.OnEvent(event);
}

void Sandbox2D::CalculateFPS(Hazel::Timestep timestep)
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
