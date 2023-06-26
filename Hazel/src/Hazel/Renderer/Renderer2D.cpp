#include "hzpch.h"
#include "Renderer2D.h"

#include "VertexArray.h"
#include "Shader.h"
#include "UniformBuffer.h"
#include "RenderCommand.h"
#include "MSDFData.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <future>

namespace Hazel
{
	struct QuadVertex
	{
		glm::vec3 Position;
		glm::vec4 Color;
		glm::vec2 TextureCoord;
		int TextureIndex;
		glm::vec2 TilingFactor;

		// Editor-Only
		int EntityID;
	};

	struct CircleVertex
	{
		glm::vec3 WorldPosition;
		glm::vec3 LocalPosition;
		glm::vec4 Color;
		float Thickness;
		float Fade;

		// Editor-Only
		int EntityID;
	};

	struct LineVertex
	{
		glm::vec3 Position;
		glm::vec4 Color;

		// Editor-Only, Not used right now as lines are not entities.
		int EntityID;
	};

	struct TextVertex
	{
		glm::vec3 Position;
		glm::vec4 Color;
		glm::vec2 TextureCoord;

		// TODO: BackgroundColor
		// Editor-Only
		int EntityID;
	};

	struct Renderer2DData
	{
		// Max for draw calls
		static constexpr uint32_t MaxQuads = 20000; // 20,000
		static constexpr uint32_t MaxVertices = MaxQuads * 4; // 80,000
		static constexpr uint32_t MaxIndices = MaxQuads * 6; // 120,000
		static constexpr uint32_t MaxTextureSlots = 32; // TODO: Render Capabilities

#pragma region Quad
		Ref<VertexArray> QuadVertexArray;
		Ref<VertexBuffer> QuadVertexBuffer;
		Ref<Shader> QuadShader;

		uint32_t QuadIndexCount = 0;
		QuadVertex* QuadVertexBufferBase = nullptr;
		QuadVertex* QuadVertexBufferPtr = nullptr;
#pragma endregion

#pragma region Circle
		Ref<VertexArray> CircleVertexArray;
		Ref<VertexBuffer> CircleVertexBuffer;
		Ref<Shader> CircleShader;

		uint32_t CircleIndexCount = 0;
		CircleVertex* CircleVertexBufferBase = nullptr;
		CircleVertex* CircleVertexBufferPtr = nullptr;
#pragma endregion

#pragma region Line
		Ref<VertexArray> LineVertexArray;
		Ref<VertexBuffer> LineVertexBuffer;
		Ref<Shader> LineShader;

		uint32_t LineVertexCount = 0;
		LineVertex* LineVertexBufferBase = nullptr;
		LineVertex* LineVertexBufferPtr = nullptr;

		float LineWidth = 2.0f;
#pragma endregion

#pragma region Text
		Ref<VertexArray> TextVertexArray;
		Ref<VertexBuffer> TextVertexBuffer;
		Ref<Shader> TextShader;

		uint32_t TextIndexCount = 0;
		TextVertex* TextVertexBufferBase = nullptr;
		TextVertex* TextVertexBufferPtr = nullptr;

		Ref<Texture2D> FontAtlasTexture;
#pragma endregion

		std::array<Ref<Texture2D>, MaxTextureSlots> TextureSlots;
		uint32_t TextureSlotIndex = 0;

		static constexpr uint8_t kQuadVertexCount = 4;
		glm::vec4 QuadVertexPositions[4];
		glm::vec2* QuadTextureCoordinates = nullptr;

		Renderer2D::Statistics Stats;

		struct CameraData
		{
			glm::mat4 ViewProjection;
		};
		CameraData CameraBuffer;
		Ref<UniformBuffer> CameraUniformBuffer;
	};

	static Renderer2DData sData;
	static std::future<void> sAsyncShaderCreation;

	void Renderer2D::Init()
	{
		HZ_PROFILE_FUNCTION();

#pragma region Quad
		sData.QuadVertexArray = VertexArray::Create();

		sData.QuadVertexBuffer = VertexBuffer::Create(Renderer2DData::MaxVertices * sizeof(QuadVertex));
		sData.QuadVertexBuffer->SetLayout(
			{
				{ ShaderDataType::Float3, "a_Position"		},
				{ ShaderDataType::Float4, "a_Color"			},
				{ ShaderDataType::Float2, "a_TextureCoord"	},
				{ ShaderDataType::Int,	  "a_TextureIndex"	},
				{ ShaderDataType::Float2, "a_TilingFactor"	},
				{ ShaderDataType::Int,	  "a_EntityID"		},
			});
		sData.QuadVertexArray->AddVertexBuffer(sData.QuadVertexBuffer);

		sData.QuadVertexBufferBase = new QuadVertex[Renderer2DData::MaxVertices];

		auto* quadIndices = new uint32_t[Renderer2DData::MaxIndices];

		uint32_t offset = 0;
		for (uint32_t i = 0; i < Renderer2DData::MaxIndices; i += 6)
		{
			quadIndices[i + 0] = offset + 0;
			quadIndices[i + 1] = offset + 1;
			quadIndices[i + 2] = offset + 2;

			quadIndices[i + 3] = offset + 2;
			quadIndices[i + 4] = offset + 3;
			quadIndices[i + 5] = offset + 0;

			offset += 4;
		}

		const auto quadIndexBuffer = IndexBuffer::Create(quadIndices, Renderer2DData::MaxIndices);
		sData.QuadVertexArray->SetIndexBuffer(quadIndexBuffer);
		delete[] quadIndices;
#pragma endregion

#pragma region Circle
		sData.CircleVertexArray = VertexArray::Create();

		sData.CircleVertexBuffer = VertexBuffer::Create(Renderer2DData::MaxVertices * sizeof(CircleVertex));
		sData.CircleVertexBuffer->SetLayout(
			{
				{ ShaderDataType::Float3, "a_WorldPosition"	},
				{ ShaderDataType::Float3, "a_LocalPosition"	},
				{ ShaderDataType::Float4, "a_Color"		},
				{ ShaderDataType::Float,  "a_Thickness"	},
				{ ShaderDataType::Float,  "a_Float"		},
				{ ShaderDataType::Int,	  "a_EntityID"	},
			});
		sData.CircleVertexArray->AddVertexBuffer(sData.CircleVertexBuffer);
		sData.CircleVertexArray->SetIndexBuffer(quadIndexBuffer); // Use Quad Index Buffer
		sData.CircleVertexBufferBase = new CircleVertex[Renderer2DData::MaxVertices];
#pragma endregion

#pragma region Line
		sData.LineVertexArray = VertexArray::Create();

		sData.LineVertexBuffer = VertexBuffer::Create(Renderer2DData::MaxVertices * sizeof(LineVertex));
		sData.LineVertexBuffer->SetLayout(
			{
				{ ShaderDataType::Float3, "a_Position"	},
				{ ShaderDataType::Float4, "a_Color"		},
				{ ShaderDataType::Int,	  "a_EntityID"	},
			});
		sData.LineVertexArray->AddVertexBuffer(sData.LineVertexBuffer);
		sData.LineVertexBufferBase = new LineVertex[Renderer2DData::MaxVertices];
#pragma endregion

#pragma region Text
		sData.TextVertexArray = VertexArray::Create();

		sData.TextVertexBuffer = VertexBuffer::Create(Renderer2DData::MaxVertices * sizeof(TextVertex));
		sData.TextVertexBuffer->SetLayout(
			{
				{ShaderDataType::Float3, "a_Position"		},
				{ShaderDataType::Float4, "a_Color"			},
				{ShaderDataType::Float2, "a_TextureCoord"	},
				{ShaderDataType::Int,	 "a_EntityID"		},
			});
		sData.TextVertexArray->AddVertexBuffer(sData.TextVertexBuffer);
		sData.TextVertexBufferBase = new TextVertex[Renderer2DData::MaxVertices];
		sData.TextVertexArray->SetIndexBuffer(quadIndexBuffer); // Use Quad Index Buffer
#pragma endregion

		// -- Shader loading
#define ASYNC 1

#if ASYNC
		LoadShadersAsync();
#else
		sData.QuadShader = Shader::Create(SHADER_PATH_QUAD);
		sData.CircleShader = Shader::Create(SHADER_PATH_CIRCLE);
		sData.LineShader = Shader::Create(SHADER_PATH_LINE);
		sData.TextShader = Shader::Create(SHADER_PATH_TEXT);
#endif // ASYNC

		sData.QuadVertexPositions[0] = {-0.5f, -0.5f, 0.0f, 1.0f};
		sData.QuadVertexPositions[1] = {0.5f, -0.5f, 0.0f, 1.0f};
		sData.QuadVertexPositions[2] = {0.5f, 0.5f, 0.0f, 1.0f};
		sData.QuadVertexPositions[3] = {-0.5f, 0.5f, 0.0f, 1.0f};

		sData.QuadTextureCoordinates = new glm::vec2[4];

		sData.CameraUniformBuffer = UniformBuffer::Create(sizeof(Renderer2DData::CameraData), 0);
	}

	void Renderer2D::Shutdown()
	{
		HZ_PROFILE_FUNCTION();

		sAsyncShaderCreation.get();
		delete[] sData.QuadVertexBufferBase;
		delete[] sData.QuadTextureCoordinates;
	}

	bool Renderer2D::BeginScene(const Camera& camera, const glm::mat4& transform)
	{
		HZ_PROFILE_FUNCTION();
		return BeginScene(camera.GetProjection() * glm::inverse(transform));
	}

	bool Renderer2D::BeginScene(const glm::mat4& viewProjection)
	{
		HZ_PROFILE_FUNCTION();

#pragma region Quad
		if (sData.QuadShader != nullptr && sData.QuadShader->IsLoadingCompleted())
		{
			sData.QuadShader->CompleteInitialization();
		}
		else
		{
			return false;
		}
#pragma endregion

#pragma region Circle
		if (sData.CircleShader != nullptr && sData.CircleShader->IsLoadingCompleted())
		{
			sData.CircleShader->CompleteInitialization();
		}
		else
		{
			return false;
		}
#pragma endregion

#pragma region Line
		if (sData.LineShader != nullptr && sData.LineShader->IsLoadingCompleted())
		{
			sData.LineShader->CompleteInitialization();
		}
		else
		{
			return false;
		}
#pragma endregion

#pragma region Text
		if (sData.TextShader != nullptr && sData.TextShader->IsLoadingCompleted())
		{
			sData.TextShader->CompleteInitialization();
		}
		else
		{
			return false;
		}
#pragma endregion

		sData.CameraBuffer.ViewProjection = viewProjection;
		sData.CameraUniformBuffer->SetData(&sData.CameraBuffer, sizeof(Renderer2DData::CameraData));
		Reset();

		return true;
	}

	void Renderer2D::EndScene()
	{
		HZ_PROFILE_FUNCTION();

		Flush();
	}

	void Renderer2D::Reset()
	{
		HZ_PROFILE_FUNCTION();

#pragma region Quad
		sData.QuadIndexCount = 0;
		sData.QuadVertexBufferPtr = sData.QuadVertexBufferBase;
#pragma endregion

#pragma region Circle
		sData.CircleIndexCount = 0;
		sData.CircleVertexBufferPtr = sData.CircleVertexBufferBase;
#pragma endregion

#pragma region Line
		sData.LineVertexCount = 0;
		sData.LineVertexBufferPtr = sData.LineVertexBufferBase;
#pragma endregion

#pragma region Text
		sData.TextIndexCount = 0;
		sData.TextVertexBufferPtr = sData.TextVertexBufferBase;
#pragma endregion

		sData.TextureSlotIndex = 0;
	}

	void Renderer2D::Flush()
	{
		HZ_PROFILE_FUNCTION();

#pragma region Quad
		if (sData.QuadIndexCount > 0)
		{
			auto dataSize = static_cast<uint32_t>(reinterpret_cast<uint8_t*>(sData.QuadVertexBufferPtr) - reinterpret_cast<uint8_t*>(sData.QuadVertexBufferBase));
			sData.QuadVertexBuffer->SetData(sData.QuadVertexBufferBase, dataSize);

			// Bind Textures
			for (uint32_t i = 0; i < sData.TextureSlotIndex; i++)
			{
				sData.TextureSlots[i]->Bind(i);
			}

			sData.QuadShader->Bind();
			RenderCommand::DrawIndexed(sData.QuadVertexArray, sData.QuadIndexCount);

			sData.Stats.DrawCalls++;
		}
#pragma endregion

#pragma region Circle
		if (sData.CircleIndexCount > 0)
		{
			auto dataSize = static_cast<uint32_t>(reinterpret_cast<uint8_t*>(sData.CircleVertexBufferPtr) - reinterpret_cast<uint8_t*>(sData.CircleVertexBufferBase));
			sData.CircleVertexBuffer->SetData(sData.CircleVertexBufferBase, dataSize);

			sData.CircleShader->Bind();
			RenderCommand::DrawIndexed(sData.CircleVertexArray, sData.CircleIndexCount);

			sData.Stats.DrawCalls++;
		}
#pragma endregion

#pragma region Line
		if (sData.LineVertexCount > 0)
		{
			const auto dataSize = static_cast<uint32_t>(reinterpret_cast<uint8_t*>(sData.LineVertexBufferPtr) - reinterpret_cast<uint8_t*>(sData.LineVertexBufferBase));
			sData.LineVertexBuffer->SetData(sData.LineVertexBufferBase, dataSize);

			sData.LineShader->Bind();
			RenderCommand::SetLineWidth(sData.LineWidth);
			RenderCommand::DrawLines(sData.LineVertexArray, sData.LineVertexCount);

			sData.Stats.DrawCalls++;
		}
#pragma endregion

#pragma region Text
		if (sData.TextIndexCount > 0)
		{
			const auto dataSize = static_cast<uint32_t>(reinterpret_cast<uint8_t*>(sData.TextVertexBufferPtr) - reinterpret_cast<uint8_t*>(sData.TextVertexBufferBase));
			sData.TextVertexBuffer->SetData(sData.TextVertexBufferBase, dataSize);

			sData.FontAtlasTexture->Bind();

			sData.TextShader->Bind();
			RenderCommand::DrawIndexed(sData.TextVertexArray, sData.TextIndexCount);

			sData.Stats.DrawCalls++;
		}
#pragma endregion
	}

#pragma region Primitive
	void Renderer2D::DrawQuad(const glm::vec2& position, const glm::vec2& size, const Color& color)
	{
		DrawQuad({position.x, position.y, 0.0f}, size, color);
	}

	void Renderer2D::DrawQuad(const glm::vec3& position, const glm::vec2& size, const Color& color)
	{
		HZ_PROFILE_FUNCTION();

		glm::mat4 transform = glm::translate(_sIdentityMatrix, position)
			* glm::scale(_sIdentityMatrix, {size.x, size.y, 1.0f});

		DrawQuad(transform, color);
	}

	void Renderer2D::DrawRotatedQuad(const glm::vec2& position, const glm::vec2& size, float rotation, const Color& color)
	{
		DrawRotatedQuad({position.x, position.y, 0.0f}, size, rotation, color);
	}

	void Renderer2D::DrawRotatedQuad(const glm::vec3& position, const glm::vec2& size, float rotation, const Color& color)
	{
		HZ_PROFILE_FUNCTION();

		glm::mat4 transform = glm::translate(_sIdentityMatrix, position)
			* glm::rotate(_sIdentityMatrix, glm::radians(rotation), {0.0f, 0.0f, 1.0f})
			* glm::scale(_sIdentityMatrix, {size.x, size.y, 1.0f});

		DrawQuad(transform, color);
	}

	void Renderer2D::DrawQuad(const glm::mat4& transform, const Color& color, int entityID)
	{
		HZ_PROFILE_FUNCTION();

		UpdateQuadData(transform, color, entityID);
	}
#pragma endregion

#pragma region Texture
	void Renderer2D::DrawQuad(const glm::vec2& position, const glm::vec2& size, const Ref<Texture2D>& texture, const glm::vec2& tilingFactor, const Color& tintColor, int entityID)
	{
		DrawQuad({position.x, position.y, 0.0f}, size, texture, tilingFactor, tintColor, entityID);
	}

	void Renderer2D::DrawQuad(const glm::vec3& position, const glm::vec2& size, const Ref<Texture2D>& texture, const glm::vec2& tilingFactor, const Color& tintColor, int entityID)
	{
		HZ_PROFILE_FUNCTION();

		glm::mat4 transform = glm::translate(_sIdentityMatrix, position)
			* glm::scale(_sIdentityMatrix, {size.x, size.y, 1.0f});

		DrawQuad(transform, texture, tilingFactor, tintColor, entityID);
	}

	void Renderer2D::DrawRotatedQuad(const glm::vec2& position, const glm::vec2& size, float rotation, const Ref<Texture2D>& texture, const glm::vec2& tilingFactor, const Color& tintColor, int entityID)
	{
		DrawRotatedQuad({position.x, position.y, 0.0f}, size, rotation, texture, tilingFactor, tintColor, entityID);
	}

	void Renderer2D::DrawRotatedQuad(const glm::vec3& position, const glm::vec2& size, float rotation, const Ref<Texture2D>& texture, const glm::vec2& tilingFactor, const Color& tintColor, int entityID)
	{
		HZ_PROFILE_FUNCTION();

		glm::mat4 transform = glm::translate(_sIdentityMatrix, position)
			* glm::rotate(_sIdentityMatrix, glm::radians(rotation), {0.0f, 0.0f, 1.0f})
			* glm::scale(_sIdentityMatrix, {size.x, size.y, 1.0f});

		DrawQuad(transform, texture, tilingFactor, tintColor, entityID);
	}

	void Renderer2D::DrawQuad(const glm::mat4& transform, const Ref<Texture2D>& texture, const glm::vec2& tilingFactor, const Color& tintColor, int entityID)
	{
		HZ_PROFILE_FUNCTION();

		if (texture == nullptr)
		{
			DrawInvalidTexture(transform);
			return;
		}

		int textureIndex = 0;

		// Check if the texture already was assigned to a texture slot.
		for (uint32_t i = 0; i < sData.TextureSlotIndex; i++)
		{
			if (sData.TextureSlots[i]->Equals(*texture))
			{
				textureIndex = i;
				break;
			}
		}

		if (textureIndex == 0)
		{
			// If all slots are taken, flush and reset.
			if (sData.TextureSlotIndex >= Renderer2DData::MaxTextureSlots)
			{
				FlushAndReset();
			}

			textureIndex = sData.TextureSlotIndex;
			sData.TextureSlots[sData.TextureSlotIndex] = texture;
			sData.TextureSlotIndex++;
		}

		sData.QuadTextureCoordinates[0] = {0.0f, 0.0f};
		sData.QuadTextureCoordinates[1] = {1.0f, 0.0f};
		sData.QuadTextureCoordinates[2] = {1.0f, 1.0f};
		sData.QuadTextureCoordinates[3] = {0.0f, 1.0f};

		UpdateQuadData(transform, tintColor, entityID, tilingFactor, textureIndex);
	}
#pragma endregion

#pragma region SubTexture
	void Renderer2D::DrawQuad(const glm::vec2& position, const glm::vec2& size, const Ref<SubTexture2D>& subTexture, const glm::vec2& tilingFactor, const Color& tintColor)
	{
		DrawQuad({position.x, position.y, 0.0f}, size, subTexture, tilingFactor, tintColor);
	}

	void Renderer2D::DrawQuad(const glm::vec3& position, const glm::vec2& size, const Ref<SubTexture2D>& subTexture, const glm::vec2& tilingFactor, const Color& tintColor)
	{
		HZ_PROFILE_FUNCTION();

		glm::mat4 transform = glm::translate(_sIdentityMatrix, position)
			* glm::scale(_sIdentityMatrix, {size.x, size.y, 1.0f});

		DrawQuad(transform, subTexture, tilingFactor, tintColor);
	}

	void Renderer2D::DrawRotatedQuad(const glm::vec2& position, const glm::vec2& size, float rotation, const Ref<SubTexture2D>& subTexture, const glm::vec2& tilingFactor, const Color& tintColor)
	{
		DrawRotatedQuad({position.x, position.y, 0.0f}, size, rotation, subTexture, tilingFactor, tintColor);
	}

	void Renderer2D::DrawRotatedQuad(const glm::vec3& position, const glm::vec2& size, float rotation, const Ref<SubTexture2D>& subTexture, const glm::vec2& tilingFactor, const Color& tintColor)
	{
		HZ_PROFILE_FUNCTION();

		glm::mat4 transform = glm::translate(_sIdentityMatrix, position)
			* glm::rotate(_sIdentityMatrix, glm::radians(rotation), {0.0f, 0.0f, 1.0f})
			* glm::scale(_sIdentityMatrix, {size.x, size.y, 1.0f});

		DrawQuad(transform, subTexture, tilingFactor, tintColor);
	}

	void Renderer2D::DrawQuad(const glm::mat4& transform, const Ref<SubTexture2D>& subTexture, const glm::vec2& tilingFactor, const Color& tintColor)
	{
		HZ_PROFILE_FUNCTION();

		if (subTexture == nullptr)
		{
			DrawInvalidTexture(transform);
			return;
		}

		auto coords = subTexture->GetTexCoords();
		auto newCoords = new glm::vec2[4];
		for (int i = 0; i < 4; i++)
		{
			newCoords[i] = coords[i];
		}
		sData.QuadTextureCoordinates = newCoords;

		int textureIndex = 0;

		// Check if the texture already was assigned to a texture slot.
		for (uint32_t i = 0; i < sData.TextureSlotIndex; i++)
		{
			if (sData.TextureSlots[i]->Equals(*subTexture->GetTexture()))
			{
				textureIndex = i;
				break;
			}
		}

		if (textureIndex == 0)
		{
			// If all slots are taken, flush and reset.
			if (sData.TextureSlotIndex >= Renderer2DData::MaxTextureSlots)
			{
				FlushAndReset();
			}

			textureIndex = sData.TextureSlotIndex;
			sData.TextureSlots[sData.TextureSlotIndex] = subTexture->GetTexture();
			sData.TextureSlotIndex++;
		}

		UpdateQuadData(transform, tintColor, -1, tilingFactor, textureIndex);
	}
#pragma endregion

	void Renderer2D::DrawSprite(const glm::mat4& transform, const SpriteRendererComponent& spriteRenderComponent, int entityID)
	{
		if (spriteRenderComponent.Texture != nullptr)
		{
			DrawQuad(transform, spriteRenderComponent.Texture, spriteRenderComponent.Tiling, spriteRenderComponent.Color, entityID);
		}
		else
		{
			DrawQuad(transform, spriteRenderComponent.Color, entityID);
		}
	}

	void Renderer2D::DrawCircle(const glm::mat4& transform, const Color& color, float thickness, float fade, int entityID)
	{
		UpdateCircleData(transform, color, entityID, thickness, fade);
	}

#pragma region Lines
	void Renderer2D::DrawLine(const glm::vec3& positionStart, const glm::vec3& positionEnd, const Color& color, int entityID)
	{
		sData.LineVertexBufferPtr->Position = positionStart;
		sData.LineVertexBufferPtr->Color = color;
		sData.LineVertexBufferPtr->EntityID = entityID;
		sData.LineVertexBufferPtr++;

		sData.LineVertexBufferPtr->Position = positionEnd;
		sData.LineVertexBufferPtr->Color = color;
		sData.LineVertexBufferPtr->EntityID = entityID;
		sData.LineVertexBufferPtr++;

		sData.LineVertexCount += 2;
	}

	void Renderer2D::DrawRect(const glm::vec3& position, const glm::vec2& size, const Color& color, int entityID)
	{
		glm::vec3 p0 = glm::vec3(position.x - size.x * 0.5f, position.y - size.y * 0.5f, position.z);
		glm::vec3 p1 = glm::vec3(position.x + size.x * 0.5f, position.y - size.y * 0.5f, position.z);
		glm::vec3 p2 = glm::vec3(position.x + size.x * 0.5f, position.y + size.y * 0.5f, position.z);
		glm::vec3 p3 = glm::vec3(position.x - size.x * 0.5f, position.y + size.y * 0.5f, position.z);

		DrawLine(p0, p1, color);
		DrawLine(p1, p2, color);
		DrawLine(p2, p3, color);
		DrawLine(p3, p0, color);
	}

	void Renderer2D::DrawRect(const glm::mat4& transform, const Color& color, int entityID)
	{
		glm::vec3 lineVertices[4];

		for (size_t i = 0; i < 4; i++)
		{
			lineVertices[i] = transform * sData.QuadVertexPositions[i];
		}

		DrawLine(lineVertices[0], lineVertices[1], color);
		DrawLine(lineVertices[1], lineVertices[2], color);
		DrawLine(lineVertices[2], lineVertices[3], color);
		DrawLine(lineVertices[3], lineVertices[0], color);
	}
#pragma endregion

	float Renderer2D::GetLineWidth()
	{
		return sData.LineWidth;
	}

	void Renderer2D::SetLineWidth(float width)
	{
		sData.LineWidth = width;
	}

	void Renderer2D::DrawString(const glm::mat4& transform, const TextComponent& textComponent, int entityID)
	{
		DrawString(transform, textComponent.Text, textComponent.FontAsset, textComponent.Color, textComponent.Kerning, textComponent.LineSpace, entityID);
	}

	void Renderer2D::DrawString(const glm::mat4& transform, const std::string& string, const Ref<Font>& font, const Color& color, float kerning, float lineSpace, int entityID)
	{
		const auto& fontGeometry = font->GetMSDFData()->FontGeometry;
		const auto& metrics = fontGeometry.getMetrics();
		Ref<Texture2D> fontAtlas = font->GetAtlasTexture();
		if (!fontAtlas)
		{
			return;
		}

		if (sData.FontAtlasTexture && sData.FontAtlasTexture != fontAtlas)
		{
			FlushAndReset();
		}

		sData.FontAtlasTexture = fontAtlas;

		double x = 0.0;
		double fsScale = 1.0 / (metrics.ascenderY - metrics.descenderY);
		double y = 0.0;

		int tabSize = 4; // How many spaces a tab equals. TODO modifiable

		for (size_t i = 0; i < string.size(); ++i)
		{
			char character = string[i];
			switch (character)
			{
			case '\r': continue; // carriage return
			case '\n': // new line
			{
				x = 0;
				y -= fsScale * (metrics.lineHeight + static_cast<double>(lineSpace));
				continue;
			}
			case '\t': // tab
			{
				const double advance = fontGeometry.getGlyph(' ')->getAdvance();
				x += fsScale * (advance + kerning) * tabSize;
				continue;
			}
			}

			const auto* glyph = fontGeometry.getGlyph(character);
			if (!glyph)
			{
				glyph = fontGeometry.getGlyph('?');
				if (!glyph)
				{
					return;
				}
			}

			double al, ab, ar, at;
			glyph->getQuadAtlasBounds(al, ab, ar, at);
			glm::vec2 textCoordMin(al, ab);
			glm::vec2 textCoordMax(ar, at);

			float texelWidth = 1.0f / static_cast<float>(fontAtlas->GetWidth());
			float texelHeight = 1.0f / static_cast<float>(fontAtlas->GetHeight());
			textCoordMin *= glm::vec2(texelWidth, texelHeight);
			textCoordMax *= glm::vec2(texelWidth, texelHeight);

			double pl, pb, pr, pt;
			glyph->getQuadPlaneBounds(pl, pb, pr, pt);
			glm::vec2 quadMin(pl, pb);
			glm::vec2 quadMax(pr, pt);
			quadMin *= fsScale;
			quadMin += glm::vec2(x, y);
			quadMax *= fsScale;
			quadMax += glm::vec2(x, y);

			// render here
			if (sData.TextIndexCount >= Renderer2DData::MaxIndices)
			{
				FlushAndReset();
			}

			sData.TextVertexBufferPtr->Position = transform * glm::vec4(quadMin, 0.0f, 1.0f);
			sData.TextVertexBufferPtr->Color = color;
			sData.TextVertexBufferPtr->TextureCoord = textCoordMin;
			sData.TextVertexBufferPtr->EntityID = entityID;
			sData.TextVertexBufferPtr++;

			sData.TextVertexBufferPtr->Position = transform * glm::vec4(quadMin.x, quadMax.y, 0.0f, 1.0f);
			sData.TextVertexBufferPtr->Color = color;
			sData.TextVertexBufferPtr->TextureCoord = {textCoordMin.x, textCoordMax.y};
			sData.TextVertexBufferPtr->EntityID = entityID;
			sData.TextVertexBufferPtr++;

			sData.TextVertexBufferPtr->Position = transform * glm::vec4(quadMax, 0.0f, 1.0f);
			sData.TextVertexBufferPtr->Color = color;
			sData.TextVertexBufferPtr->TextureCoord = textCoordMax;
			sData.TextVertexBufferPtr->EntityID = entityID;
			sData.TextVertexBufferPtr++;

			sData.TextVertexBufferPtr->Position = transform * glm::vec4(quadMax.x, quadMin.y, 0.0f, 1.0f);
			sData.TextVertexBufferPtr->Color = color;
			sData.TextVertexBufferPtr->TextureCoord = {textCoordMax.x, textCoordMin.y};
			sData.TextVertexBufferPtr->EntityID = entityID;
			sData.TextVertexBufferPtr++;

			sData.TextIndexCount += 6;
			sData.Stats.QuadCount++;

			if (i < string.size() - 1)
			{
				double advance = glyph->getAdvance();
				const char nextCharacter = string[i + 1];
				fontGeometry.getAdvance(advance, character, nextCharacter)
					|| fontGeometry.getAdvance(advance, character, ' '); // If we can't find the advance with the next character replace it by a space.
				x += fsScale * (advance + static_cast<double>(kerning));
			}
		}
	}

	void Renderer2D::ResetStats()
	{
		memset(&sData.Stats, 0, sizeof(Statistics));
	}

	Renderer2D::Statistics Renderer2D::GetStats()
	{
		return sData.Stats;
	}

	bool Renderer2D::IsReady()
	{
		return sData.QuadShader && sData.CircleShader && sData.LineShader && sData.TextShader;
	}

	void Renderer2D::ReloadShader(RendererShader rendererShader)
	{
		if (!IsReady())
		{
			HZ_CORE_LERROR("Wait for shaders loading completed");
			return;
		}

		HZ_CORE_LINFO("Reloading Shader");

		switch (rendererShader)
		{
		case Hazel::QUAD:
			sData.QuadShader = nullptr;
			sAsyncShaderCreation = std::async(std::launch::async, []()
			{
				sData.QuadShader = Shader::Create(SHADER_PATH_QUAD, true);
			});
			break;
		case Hazel::CIRCLE:
			sData.CircleShader = nullptr;
			sAsyncShaderCreation = std::async(std::launch::async, []()
			{
				sData.CircleShader = Shader::Create(SHADER_PATH_CIRCLE, true);
			});
			break;
		case Hazel::LINE:
			sData.LineShader = nullptr;
			sAsyncShaderCreation = std::async(std::launch::async, []()
			{
				sData.LineShader = Shader::Create(SHADER_PATH_LINE, true);
			});
			break;
		case Hazel::TEXT:
			sData.TextShader = nullptr;
			sAsyncShaderCreation = std::async(std::launch::async, []()
			{
				sData.TextShader = Shader::Create(SHADER_PATH_TEXT, true);
			});
			break;
		default:
			HZ_CORE_LERROR("No shader of that type exist.");
			return;
		}
	}

	void Renderer2D::LoadShadersAsync()
	{
		sData.QuadShader = nullptr;
		sData.CircleShader = nullptr;
		sData.LineShader = nullptr;
		sData.TextShader = nullptr;

		sAsyncShaderCreation = std::async(std::launch::async, []
		{
			sData.QuadShader = Shader::Create(SHADER_PATH_QUAD);
			sData.CircleShader = Shader::Create(SHADER_PATH_CIRCLE);
			sData.LineShader = Shader::Create(SHADER_PATH_LINE);
			sData.TextShader = Shader::Create(SHADER_PATH_TEXT);
		});
	}

	void Renderer2D::FlushAndReset()
	{
		HZ_PROFILE_FUNCTION();

		EndScene();
		Reset();
	}

	void Renderer2D::UpdateQuadData(const glm::mat4& transform, const Color& color, int entityID, const glm::vec2& tilingFactor, int textureIndex)
	{
		HZ_PROFILE_FUNCTION();

		if (sData.QuadIndexCount >= Renderer2DData::MaxIndices)
		{
			FlushAndReset();
		}

		for (uint32_t i = 0; i < Renderer2DData::kQuadVertexCount; i++)
		{
			sData.QuadVertexBufferPtr->Position = transform * sData.QuadVertexPositions[i];
			sData.QuadVertexBufferPtr->Color = color;
			sData.QuadVertexBufferPtr->TextureCoord = sData.QuadTextureCoordinates[i];
			sData.QuadVertexBufferPtr->TextureIndex = textureIndex;
			sData.QuadVertexBufferPtr->TilingFactor = tilingFactor;
			sData.QuadVertexBufferPtr->EntityID = entityID;
			sData.QuadVertexBufferPtr++;
		}

		sData.QuadIndexCount += 6;
		sData.Stats.QuadCount++;
	}

	void Renderer2D::UpdateCircleData(const glm::mat4& transform, const Color& color, int entityID, float thickness, float fade)
	{
		HZ_PROFILE_FUNCTION();

		if (sData.CircleIndexCount >= Renderer2DData::MaxIndices)
		{
			FlushAndReset();
		}

		for (const auto& quadVertexPosition : sData.QuadVertexPositions)
		{
			sData.CircleVertexBufferPtr->WorldPosition = transform * quadVertexPosition;
			sData.CircleVertexBufferPtr->LocalPosition = quadVertexPosition * 2.0f;
			sData.CircleVertexBufferPtr->Color = color;
			sData.CircleVertexBufferPtr->Thickness = thickness;
			sData.CircleVertexBufferPtr->Fade = fade;
			sData.CircleVertexBufferPtr->EntityID = entityID;
			sData.CircleVertexBufferPtr++;
		}

		sData.CircleIndexCount += 6;
		sData.Stats.QuadCount++;
	}

	void Renderer2D::DrawInvalidTexture(const glm::mat4& transform)
	{
		DrawQuad(transform, Color::Pink);
	}
}
