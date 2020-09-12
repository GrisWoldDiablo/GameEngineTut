#include "hzpch.h"
#include "Renderer2D.h"

#include "VertexArray.h"
#include "Shader.h"
#include "RenderCommand.h"

#include <glm/gtc/matrix_transform.hpp>

namespace Hazel
{
	struct Renderer2DStorage
	{
		Ref<VertexArray> QuadVertexArray;
		Ref<Shader> TextureShader;
		Ref<Texture2D> WhiteTexture;
	};

	static Renderer2DStorage* sData;

	void Renderer2D::Init()
	{
		HZ_PROFILE_FUNCTION();

		sData = new Renderer2DStorage();
		sData->QuadVertexArray = VertexArray::Create();

		float squareVertices[4 * 5] =
		{
			-0.5f, -0.5f, 0.0f, 0.0f, 0.0f,
			 0.5f, -0.5f, 0.0f, 1.0f, 0.0f,
			 0.5f,  0.5f, 0.0f, 1.0f, 1.0f,
			-0.5f,  0.5f, 0.0f, 0.0f, 1.0f
		};

		Ref<VertexBuffer> squareVertexBuffer;
		squareVertexBuffer = VertexBuffer::Create(squareVertices, sizeof(squareVertices));
		squareVertexBuffer->SetLayout(
			{
				{ ShaderDataType::Float3, "a_Position" },
				{ ShaderDataType::Float2, "a_TexCoord" },
			});
		sData->QuadVertexArray->AddVertexBuffer(squareVertexBuffer);

		uint32_t squareIndices[6] = { 0, 1, 2, 2, 3, 0 };
		Ref<IndexBuffer> squareIndexBuffer;
		squareIndexBuffer = IndexBuffer::Create(squareIndices, sizeof(squareIndices) / sizeof(uint32_t));
		sData->QuadVertexArray->SetIndexBuffer(squareIndexBuffer);
		// -- Square

		// Create a white texture, so we can render flat color using the texture shader.
		sData->WhiteTexture = Texture2D::Create(1, 1);
		uint32_t whiteTextureData = 0xffffffff;
		sData->WhiteTexture->SetData(&whiteTextureData, sizeof(uint32_t));

		// -- Shaders
		sData->TextureShader = Shader::Create("assets/shaders/Texture.glsl");
		sData->TextureShader->Bind();
		sData->TextureShader->SetInt("u_Texture", 0);
	}

	void Renderer2D::Shutdown()
	{
		HZ_PROFILE_FUNCTION();

		delete sData;
	}

	void Renderer2D::BeginScene(OrthographicCamera& camera)
	{
		HZ_PROFILE_FUNCTION();

		sData->TextureShader->Bind();
		sData->TextureShader->SetMat4("u_ViewProjection", camera.GetViewProjectionMatrix());
	}

	void Renderer2D::EndScene()
	{
		HZ_PROFILE_FUNCTION();
	}

	void Renderer2D::DrawQuad(const glm::vec2& position, const glm::vec2& size, const Color& color)
	{
		DrawQuad({ position.x, position.y, 0.0f }, size, color);
	}

	void Renderer2D::DrawQuad(const glm::vec3& position, const glm::vec2& size, const Color& color)
	{
		HZ_PROFILE_FUNCTION();

		sData->TextureShader->SetFloat4("u_Color", color);
		// No need to set Tiling factor if there is no texture
		//sData->TextureShader->SetFloat2("u_TilingFactor", glm::vec2(1.0f));
		sData->WhiteTexture->Bind();

		auto transform = glm::translate(glm::mat4(1.0f), position) * glm::scale(glm::mat4(1.0f), { size.x,size.y,1.0f });
		sData->TextureShader->SetMat4("u_Transform", transform);

		sData->QuadVertexArray->Bind();
		RenderCommand::DrawIndexed(sData->QuadVertexArray);
	}

	void Renderer2D::DrawQuad(const glm::vec2& position, const glm::vec2& size, const Ref<Texture2D> texture, const glm::vec2& tilingFactor, const Color& tintColor)
	{
		DrawQuad({ position.x, position.y, 0.0f }, size, texture, tilingFactor, tintColor);
	}

	void Renderer2D::DrawQuad(const glm::vec3& position, const glm::vec2& size, const Ref<Texture2D> texture, const glm::vec2& tilingFactor, const Color& tintColor)
	{
		HZ_PROFILE_FUNCTION();

		sData->TextureShader->SetFloat4("u_Color", tintColor);
		sData->TextureShader->SetFloat2("u_TilingFactor", tilingFactor);
		texture->Bind();

		auto transform = glm::translate(glm::mat4(1.0f), position) * glm::scale(glm::mat4(1.0f), { size.x,size.y,1.0f });
		sData->TextureShader->SetMat4("u_Transform", transform);

		sData->QuadVertexArray->Bind();
		RenderCommand::DrawIndexed(sData->QuadVertexArray);
	}

	void Renderer2D::DrawRotatedQuad(const glm::vec2& position, const glm::vec2& size, float rotation, const Color& color)
	{
		DrawRotatedQuad({ position.x, position.y, 0.0f }, size, rotation, color);
	}

	void Renderer2D::DrawRotatedQuad(const glm::vec3& position, const glm::vec2& size, float rotation, const Color& color)
	{
		sData->TextureShader->SetFloat4("u_Color", color);
		// No need to set Tiling factor if there is no texture
		//sData->TextureShader->SetFloat2("u_TilingFactor", glm::vec2(1.0f));
		sData->WhiteTexture->Bind();

		auto transform = glm::translate(glm::mat4(1.0f), position)
			* glm::rotate(glm::mat4(1.0f), glm::radians(rotation), { 0.0f, 0.0f, 1.0f })
			* glm::scale(glm::mat4(1.0f), { size.x,size.y,1.0f });
		sData->TextureShader->SetMat4("u_Transform", transform);

		sData->QuadVertexArray->Bind();
		RenderCommand::DrawIndexed(sData->QuadVertexArray);
	}

	void Renderer2D::DrawRotatedQuad(const glm::vec2& position, const glm::vec2& size, float rotation, const Ref<Texture2D> texture, const glm::vec2& tilingFactor, const Color& tintColor)
	{
		DrawRotatedQuad({ position.x, position.y, 0.0f }, size, rotation, texture, tilingFactor, tintColor);
	}

	void Renderer2D::DrawRotatedQuad(const glm::vec3& position, const glm::vec2& size, float rotation, const Ref<Texture2D> texture, const glm::vec2& tilingFactor, const Color& tintColor)
	{
		sData->TextureShader->SetFloat4("u_Color", tintColor);
		sData->TextureShader->SetFloat2("u_TilingFactor", tilingFactor);
		texture->Bind();

		auto transform = glm::translate(glm::mat4(1.0f), position) 
			* glm::rotate(glm::mat4(1.0f), glm::radians(rotation), { 0.0f, 0.0f, 1.0f })
			* glm::scale(glm::mat4(1.0f), { size.x,size.y,1.0f });
		sData->TextureShader->SetMat4("u_Transform", transform);

		sData->QuadVertexArray->Bind();
		RenderCommand::DrawIndexed(sData->QuadVertexArray);
	}
}
