#include "hzpch.h"
#include "Renderer2D.h"

#include "VertexArray.h"
#include "Shader.h"
#include "RenderCommand.h"

#include <glm/gtc/matrix_transform.hpp>

namespace Hazel
{
	struct QuadVertex
	{
		glm::vec3 Position;
		glm::vec4 Color;
		glm::vec2 TexCoord;
		// TODO: textureId
	};

	struct Renderer2DData
	{
		// Max for draw calls
		const uint32_t MaxQuads = 20000; // 20,000
		const uint32_t MaxVertices = MaxQuads * 4; // 80,000
		const uint32_t MaxIndices = MaxQuads * 6; // 120,000

		Ref<VertexArray> QuadVertexArray;
		Ref<VertexBuffer> QuadVertexBuffer;
		Ref<Shader> TextureShader;
		Ref<Texture2D> WhiteTexture;

		uint32_t QuadIndexCount = 0;
		QuadVertex* QuadVertexBufferBase = nullptr;
		QuadVertex* QuadVertexBufferPtr = nullptr;
	};

	static Renderer2DData sData;

	void Renderer2D::Init()
	{
		HZ_PROFILE_FUNCTION();

		sData.QuadVertexArray = VertexArray::Create();

		sData.QuadVertexBuffer = VertexBuffer::Create(sData.MaxVertices * sizeof(QuadVertex));
		sData.QuadVertexBuffer->SetLayout(
			{
				{ ShaderDataType::Float3, "a_Position" },
				{ ShaderDataType::Float4, "a_Color" },
				{ ShaderDataType::Float2, "a_TexCoord" },
			});
		sData.QuadVertexArray->AddVertexBuffer(sData.QuadVertexBuffer);

		sData.QuadVertexBufferBase = new QuadVertex[sData.MaxVertices];

		auto quadIndices = new uint32_t[sData.MaxIndices];

		uint32_t offset = 0;
		for (uint32_t i = 0; i < sData.MaxIndices; i += 6)
		{
			quadIndices[i + 0] = offset + 0;
			quadIndices[i + 1] = offset + 1;
			quadIndices[i + 2] = offset + 2;

			quadIndices[i + 3] = offset + 2;
			quadIndices[i + 4] = offset + 3;
			quadIndices[i + 5] = offset + 0;

			offset += 4;
		}

		auto quadIndexBuffer = IndexBuffer::Create(quadIndices, sData.MaxIndices);
		sData.QuadVertexArray->SetIndexBuffer(quadIndexBuffer);
		delete[] quadIndices;

		// Create a white texture, so we can render flat color using the texture shader.
		sData.WhiteTexture = Texture2D::Create(1, 1);
		uint32_t whiteTextureData = 0xffffffff;
		sData.WhiteTexture->SetData(&whiteTextureData, sizeof(uint32_t));

		// -- Shaders
		sData.TextureShader = Shader::Create("assets/shaders/Texture.glsl");
		sData.TextureShader->Bind();
		sData.TextureShader->SetInt("u_Texture", 0);
	}

	void Renderer2D::Shutdown()
	{
		HZ_PROFILE_FUNCTION();
	}

	void Renderer2D::BeginScene(OrthographicCamera& camera)
	{
		HZ_PROFILE_FUNCTION();

		sData.TextureShader->Bind();
		sData.TextureShader->SetMat4("u_ViewProjection", camera.GetViewProjectionMatrix());

		sData.QuadIndexCount = 0;
		sData.QuadVertexBufferPtr = sData.QuadVertexBufferBase;
	}

	void Renderer2D::EndScene()
	{
		HZ_PROFILE_FUNCTION();

		auto dataSize = (uint32_t)((uint8_t*)sData.QuadVertexBufferPtr - (uint8_t*)sData.QuadVertexBufferBase);
		sData.QuadVertexBuffer->SetData(sData.QuadVertexBufferBase, dataSize);

		Flush();
	}

	void Renderer2D::Flush()
	{
		HZ_PROFILE_FUNCTION();

		RenderCommand::DrawIndexed(sData.QuadVertexArray, sData.QuadIndexCount);
	}

	void Renderer2D::DrawQuad(const glm::vec2& position, const glm::vec2& size, const Color& color)
	{
		DrawQuad({ position.x, position.y, 0.0f }, size, color);
	}

	void Renderer2D::DrawQuad(const glm::vec3& position, const glm::vec2& size, const Color& color)
	{
		HZ_PROFILE_FUNCTION();
		
		sData.QuadVertexBufferPtr->Position = position;
		sData.QuadVertexBufferPtr->Color = color;
		sData.QuadVertexBufferPtr->TexCoord = { 0.0f, 0.0f };
		sData.QuadVertexBufferPtr++;

		sData.QuadVertexBufferPtr->Position = { position.x + size.x, position.y, 0.0f };
		sData.QuadVertexBufferPtr->Color = color;
		sData.QuadVertexBufferPtr->TexCoord = { 1.0f, 0.0f };
		sData.QuadVertexBufferPtr++;

		sData.QuadVertexBufferPtr->Position = { position.x + size.x, position.y + size.y, 0.0f };
		sData.QuadVertexBufferPtr->Color = color;
		sData.QuadVertexBufferPtr->TexCoord = { 1.0f, 1.0f };
		sData.QuadVertexBufferPtr++;

		sData.QuadVertexBufferPtr->Position = { position.x, position.y + size.y, 0.0f };
		sData.QuadVertexBufferPtr->Color = color;
		sData.QuadVertexBufferPtr->TexCoord = { 0.0f, 1.0f };
		sData.QuadVertexBufferPtr++;

		sData.QuadIndexCount += 6;

		/* Removed since using batch renderer
		// No need to set Tiling factor if there is no texture
		//sData.TextureShader->SetFloat2("u_TilingFactor", glm::vec2(1.0f));
		sData.WhiteTexture->Bind();

		auto transform = glm::translate(glm::mat4(1.0f), position)
			* glm::scale(glm::mat4(1.0f), { size.x,size.y,1.0f });
		sData.TextureShader->SetMat4("u_Transform", transform);

		sData.QuadVertexArray->Bind();
		RenderCommand::DrawIndexed(sData.QuadVertexArray);
		*/
	}

	void Renderer2D::DrawQuad(const glm::vec2& position, const glm::vec2& size, const Ref<Texture2D> texture, const glm::vec2& tilingFactor, const Color& tintColor)
	{
		DrawQuad({ position.x, position.y, 0.0f }, size, texture, tilingFactor, tintColor);
	}

	void Renderer2D::DrawQuad(const glm::vec3& position, const glm::vec2& size, const Ref<Texture2D> texture, const glm::vec2& tilingFactor, const Color& tintColor)
	{
		HZ_PROFILE_FUNCTION();

		sData.TextureShader->SetFloat4("u_Color", tintColor);
		sData.TextureShader->SetFloat2("u_TilingFactor", tilingFactor);
		texture->Bind();

		auto transform = glm::translate(glm::mat4(1.0f), position) * glm::scale(glm::mat4(1.0f), { size.x,size.y,1.0f });
		sData.TextureShader->SetMat4("u_Transform", transform);

		sData.QuadVertexArray->Bind();
		RenderCommand::DrawIndexed(sData.QuadVertexArray);
	}

	void Renderer2D::DrawRotatedQuad(const glm::vec2& position, const glm::vec2& size, float rotation, const Color& color)
	{
		DrawRotatedQuad({ position.x, position.y, 0.0f }, size, rotation, color);
	}

	void Renderer2D::DrawRotatedQuad(const glm::vec3& position, const glm::vec2& size, float rotation, const Color& color)
	{
		sData.TextureShader->SetFloat4("u_Color", color);
		// No need to set Tiling factor if there is no texture
		//sData.TextureShader->SetFloat2("u_TilingFactor", glm::vec2(1.0f));
		sData.WhiteTexture->Bind();

		auto transform = glm::translate(glm::mat4(1.0f), position)
			* glm::rotate(glm::mat4(1.0f), glm::radians(rotation), { 0.0f, 0.0f, 1.0f })
			* glm::scale(glm::mat4(1.0f), { size.x,size.y,1.0f });
		sData.TextureShader->SetMat4("u_Transform", transform);

		sData.QuadVertexArray->Bind();
		RenderCommand::DrawIndexed(sData.QuadVertexArray);
	}

	void Renderer2D::DrawRotatedQuad(const glm::vec2& position, const glm::vec2& size, float rotation, const Ref<Texture2D> texture, const glm::vec2& tilingFactor, const Color& tintColor)
	{
		DrawRotatedQuad({ position.x, position.y, 0.0f }, size, rotation, texture, tilingFactor, tintColor);
	}

	void Renderer2D::DrawRotatedQuad(const glm::vec3& position, const glm::vec2& size, float rotation, const Ref<Texture2D> texture, const glm::vec2& tilingFactor, const Color& tintColor)
	{
		sData.TextureShader->SetFloat4("u_Color", tintColor);
		sData.TextureShader->SetFloat2("u_TilingFactor", tilingFactor);
		texture->Bind();

		auto transform = glm::translate(glm::mat4(1.0f), position)
			* glm::rotate(glm::mat4(1.0f), glm::radians(rotation), { 0.0f, 0.0f, 1.0f })
			* glm::scale(glm::mat4(1.0f), { size.x,size.y,1.0f });
		sData.TextureShader->SetMat4("u_Transform", transform);

		sData.QuadVertexArray->Bind();
		RenderCommand::DrawIndexed(sData.QuadVertexArray);
	}
}
