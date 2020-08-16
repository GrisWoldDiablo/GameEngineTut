#include "hzpch.h"
#include "Renderer2D.h"

#include "VertexArray.h"
#include "Shader.h"
#include "RenderCommand.h"

#include "Platform/OpenGL/OpenGLShader.h"

namespace Hazel
{
	struct Renderer2DStorage
	{
		Ref<VertexArray> QuadVertexArray;
		Ref<Shader> FlatColorShader;
	};

	static Renderer2DStorage* sData;

	void Renderer2D::Init()
	{
		sData = new Renderer2DStorage();
		sData->QuadVertexArray = VertexArray::Create();

		float squareVertices[4 * 3] =
		{
			-0.5f, -0.5f, 0.0f,
			 0.5f, -0.5f, 0.0f,
			 0.5f,  0.5f, 0.0f,
			-0.5f,  0.5f, 0.0f
		};

		Ref<VertexBuffer> squareVertexBuffer;
		squareVertexBuffer = VertexBuffer::Create(squareVertices, sizeof(squareVertices));
		squareVertexBuffer->SetLayout(
			{
				{ ShaderDataType::Float3, "a_Position" },
			});
		sData->QuadVertexArray->AddVertexBuffer(squareVertexBuffer);

		uint32_t squareIndices[6] = { 0, 1, 2, 2, 3, 0 };
		Ref<IndexBuffer> squareIndexBuffer;
		squareIndexBuffer = IndexBuffer::Create(squareIndices, sizeof(squareIndices) / sizeof(uint32_t));
		sData->QuadVertexArray->SetIndexBuffer(squareIndexBuffer);

		sData->FlatColorShader = Shader::Create("assets/shaders/FlatColor.glsl");
		// -- Square
	}

	void Renderer2D::Shutdown()
	{
		delete sData;
	}

	void Renderer2D::BeginScene(OrthographicCamera& camera)
	{
		std::dynamic_pointer_cast<OpenGLShader>(sData->FlatColorShader)->Bind();
		std::dynamic_pointer_cast<OpenGLShader>(sData->FlatColorShader)->UploadUniformMat4("u_ViewProjection", camera.GetViewProjectionMatrix());
		std::dynamic_pointer_cast<OpenGLShader>(sData->FlatColorShader)->UploadUniformMat4("u_Transform", glm::mat4(1.0f));
	}

	void Renderer2D::EndScene()
	{

	}

	void Renderer2D::DrawQuad(const glm::vec2& position, const glm::vec2& size, glm::vec4 color)
	{
		DrawQuad({ position.x, position.y, 0.0f }, size, color);
	}

	void Renderer2D::DrawQuad(const glm::vec3& position, const glm::vec2& size, glm::vec4 color)
	{
		std::dynamic_pointer_cast<OpenGLShader>(sData->FlatColorShader)->Bind();
		std::dynamic_pointer_cast<Hazel::OpenGLShader>(sData->FlatColorShader)->UploadUniformFloat4("u_Color", color);

		sData->QuadVertexArray->Bind();
		RenderCommand::DrawIndexed(sData->QuadVertexArray);
	}
}
