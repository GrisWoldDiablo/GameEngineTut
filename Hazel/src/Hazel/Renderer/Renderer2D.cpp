#include "hzpch.h"
#include "Renderer2D.h"

#include "VertexArray.h"
#include "Shader.h"
#include "RenderCommand.h"

namespace Hazel
{
	struct QuadVertex
	{
		glm::vec3 Position;
		glm::vec4 Color;
		glm::vec2 TextureCoord;
		float TextureIndex;
		glm::vec2 TilingFactor;
	};

	struct Renderer2DData
	{
		// Max for draw calls
		static const uint32_t MaxQuads = 20000; // 20,000
		static const uint32_t MaxVertices = MaxQuads * 4; // 80,000
		static const uint32_t MaxIndices = MaxQuads * 6; // 120,000
		static const uint32_t MaxTextureSlots = 32; // TODO: Render Capabilities

		Ref<VertexArray> QuadVertexArray;
		Ref<VertexBuffer> QuadVertexBuffer;
		Ref<Shader> TextureShader;
		Ref<Texture2D> WhiteTexture;

		uint32_t QuadIndexCount = 0;
		QuadVertex* QuadVertexBufferBase = nullptr;
		QuadVertex* QuadVertexBufferPtr = nullptr;

		std::array<Ref<Texture2D>, MaxTextureSlots> TextureSlots;
		uint32_t TextureSlotIndex = 1; // Default index is 1 because, index 0 is White Texture.

		static const uint8_t QuadVertexCount = 4;
		glm::vec4 QuadVertexPositions[4];
		glm::vec2 QuadTextureCoordinates[4];

		Renderer2D::Statistics Stats;
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
				{ ShaderDataType::Float2, "a_TextureCoord" },
				{ ShaderDataType::Float, "a_TextureIndex" },
				{ ShaderDataType::Float2, "a_TilingFactor" },
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

		int32_t samplers[sData.MaxTextureSlots];
		for (uint32_t i = 0; i < sData.MaxTextureSlots; i++)
		{
			samplers[i] = i;
		}

		// -- Shaders
		sData.TextureShader = Shader::Create("assets/shaders/Texture.glsl");
		sData.TextureShader->Bind();
		sData.TextureShader->SetIntArray("u_Texture", samplers, sData.MaxTextureSlots);

		sData.TextureSlots[0] = sData.WhiteTexture;

		sData.QuadVertexPositions[0] = { -0.5f,-0.5f, 0.0f, 1.0f };
		sData.QuadVertexPositions[1] = { 0.5f,-0.5f, 0.0f, 1.0f };
		sData.QuadVertexPositions[2] = { 0.5f, 0.5f, 0.0f, 1.0f };
		sData.QuadVertexPositions[3] = { -0.5f, 0.5f, 0.0f, 1.0f };

		constexpr float x = 7.0f;
		constexpr float y = 6.0f;
		constexpr float sheetWidth = 2560.0f;
		constexpr float sheetHeight = 1664.0f;
		constexpr float spriteWidth = 128.0f;
		constexpr float spriteHeight = 128.0f;
		sData.QuadTextureCoordinates[0] = { x * spriteWidth / sheetWidth, y * spriteHeight / sheetHeight };
		sData.QuadTextureCoordinates[1] = { (x + 1) * spriteWidth / sheetWidth, y * spriteHeight / sheetHeight };
		sData.QuadTextureCoordinates[2] = { (x + 1) * spriteWidth / sheetWidth, (y + 1) * spriteHeight / sheetHeight };
		sData.QuadTextureCoordinates[3] = { x * spriteWidth / sheetWidth, (y + 1) * spriteHeight / sheetHeight };

		//sData.QuadTextureCoordinates[0] = { 0.0f, 0.0f };
		//sData.QuadTextureCoordinates[1] = { 1.0f, 0.0f };
		//sData.QuadTextureCoordinates[2] = { 1.0f, 1.0f };
		//sData.QuadTextureCoordinates[3] = { 0.0f, 1.0f };
	}

	void Renderer2D::Shutdown()
	{
		HZ_PROFILE_FUNCTION();

		delete[] sData.QuadVertexBufferBase;
	}

	void Renderer2D::BeginScene(OrthographicCamera& camera, bool isGrayscale)
	{
		HZ_PROFILE_FUNCTION();

		sData.TextureShader->Bind();
		sData.TextureShader->SetMat4("u_ViewProjection", camera.GetViewProjectionMatrix());
		sData.TextureShader->SetBool("u_IsGrayscale", isGrayscale);

		Reset();
	}

	void Renderer2D::EndScene()
	{
		HZ_PROFILE_FUNCTION();

		auto dataSize = (uint32_t)((uint8_t*)sData.QuadVertexBufferPtr - (uint8_t*)sData.QuadVertexBufferBase);
		sData.QuadVertexBuffer->SetData(sData.QuadVertexBufferBase, dataSize);

		Flush();
	}

	void Renderer2D::Reset()
	{
		HZ_PROFILE_FUNCTION();

		sData.QuadIndexCount = 0;
		sData.QuadVertexBufferPtr = sData.QuadVertexBufferBase;

		sData.TextureSlotIndex = 1;
	}

	void Renderer2D::Flush()
	{
		HZ_PROFILE_FUNCTION();

		if (sData.QuadIndexCount == 0)
		{
			return;
		}

		// Bind Textures
		for (uint32_t i = 0; i < sData.TextureSlotIndex; i++)
		{
			sData.TextureSlots[i]->Bind(i);
		}

		RenderCommand::DrawIndexed(sData.QuadVertexArray, sData.QuadIndexCount);

		sData.Stats.DrawCalls++;
	}

	void Renderer2D::DrawQuad(const glm::vec2& position, const glm::vec2& size, const Color& color)
	{
		DrawQuad({ position.x, position.y, 0.0f }, size, color);
	}

	void Renderer2D::DrawQuad(const glm::vec3& position, const glm::vec2& size, const Color& color)
	{
		HZ_PROFILE_FUNCTION();

		glm::mat4 transform = glm::translate(_sIdentityMatrix, position)
			* glm::scale(_sIdentityMatrix, { size.x,size.y,1.0f });

		DrawQuad(transform, color);
	}

	void Renderer2D::DrawQuad(const glm::vec2& position, const glm::vec2& size, const Ref<Texture2D>& texture, const glm::vec2& tilingFactor, const Color& tintColor)
	{
		DrawQuad({ position.x, position.y, 0.0f }, size, texture, tilingFactor, tintColor);
	}

	void Renderer2D::DrawQuad(const glm::vec3& position, const glm::vec2& size, const Ref<Texture2D>& texture, const glm::vec2& tilingFactor, const Color& tintColor)
	{
		HZ_PROFILE_FUNCTION();

		glm::mat4 transform = glm::translate(_sIdentityMatrix, position)
			* glm::scale(_sIdentityMatrix, { size.x,size.y,1.0f });

		DrawQuad(transform, texture, tilingFactor, tintColor);
	}

	void Renderer2D::DrawQuad(const glm::mat4& transform, const Color& color)
	{
		HZ_PROFILE_FUNCTION();

		UpdateData(transform, color);
	}

	void Renderer2D::DrawQuad(const glm::mat4& transform, const Ref<Texture2D>& texture, const glm::vec2& tilingFactor, const Color& tintColor)
	{
		HZ_PROFILE_FUNCTION();

		float textureIndex = 0.0f;

		// Check if the texture already was assigned to a texture slot.
		for (uint32_t i = 1; i < sData.TextureSlotIndex; i++)
		{
			if (sData.TextureSlots[i]->Equals(*texture))
			{
				textureIndex = (float)i;
				break;
			}
		}

		if (textureIndex == 0.0f)
		{
			// If all slots are taken, flush and reset.
			if (sData.TextureSlotIndex >= Renderer2DData::MaxTextureSlots)
			{
				FlushAndReset();
			}

			textureIndex = (float)sData.TextureSlotIndex;
			sData.TextureSlots[sData.TextureSlotIndex] = texture;
			sData.TextureSlotIndex++;
		}

		UpdateData(transform, tintColor, tilingFactor, textureIndex);
	}

	void Renderer2D::DrawRotatedQuad(const glm::vec2& position, const glm::vec2& size, float rotation, const Color& color)
	{
		DrawRotatedQuad({ position.x, position.y, 0.0f }, size, rotation, color);
	}

	void Renderer2D::DrawRotatedQuad(const glm::vec3& position, const glm::vec2& size, float rotation, const Color& color)
	{
		HZ_PROFILE_FUNCTION();

		glm::mat4 transform = glm::translate(_sIdentityMatrix, position)
			* glm::rotate(_sIdentityMatrix, glm::radians(rotation), { 0.0f,0.0f,1.0f })
			* glm::scale(_sIdentityMatrix, { size.x,size.y,1.0f });

		DrawQuad(transform, color);
	}

	void Renderer2D::DrawRotatedQuad(const glm::vec2& position, const glm::vec2& size, float rotation, const Ref<Texture2D>& texture, const glm::vec2& tilingFactor, const Color& tintColor)
	{
		DrawRotatedQuad({ position.x, position.y, 0.0f }, size, rotation, texture, tilingFactor, tintColor);
	}

	void Renderer2D::DrawRotatedQuad(const glm::vec3& position, const glm::vec2& size, float rotation, const Ref<Texture2D>& texture, const glm::vec2& tilingFactor, const Color& tintColor)
	{
		HZ_PROFILE_FUNCTION();

		glm::mat4 transform = glm::translate(_sIdentityMatrix, position)
			* glm::rotate(_sIdentityMatrix, glm::radians(rotation), { 0.0f,0.0f,1.0f })
			* glm::scale(_sIdentityMatrix, { size.x,size.y,1.0f });

		DrawQuad(transform, texture, tilingFactor, tintColor);
	}

	void Renderer2D::ResetStats()
	{
		memset(&sData.Stats, 0, sizeof(Statistics));
	}

	Renderer2D::Statistics Renderer2D::GetStats()
	{
		return sData.Stats;
	}

	void Renderer2D::FlushAndReset()
	{
		HZ_PROFILE_FUNCTION();

		EndScene();
		Reset();
	}

	void Renderer2D::UpdateData(const glm::mat4& transform, const Hazel::Color& color, const glm::vec2& tilingFactor, float textureIndex)
	{
		HZ_PROFILE_FUNCTION();

		if (sData.QuadIndexCount >= Renderer2DData::MaxIndices)
		{
			FlushAndReset();
		}

		for (uint32_t i = 0; i < Renderer2DData::QuadVertexCount; i++)
		{
			sData.QuadVertexBufferPtr->Position = transform * sData.QuadVertexPositions[i];
			sData.QuadVertexBufferPtr->Color = color;
			sData.QuadVertexBufferPtr->TextureCoord = sData.QuadTextureCoordinates[i];
			sData.QuadVertexBufferPtr->TextureIndex = textureIndex;
			sData.QuadVertexBufferPtr->TilingFactor = tilingFactor;
			sData.QuadVertexBufferPtr++;
		}

		sData.QuadIndexCount += 6;
		sData.Stats.QuadCount++;
	}
}
