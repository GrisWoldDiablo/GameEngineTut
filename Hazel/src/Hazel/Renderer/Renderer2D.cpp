#include "hzpch.h"
#include "Renderer2D.h"

#include "VertexArray.h"
#include "Shader.h"
#include "UniformBuffer.h"
#include "RenderCommand.h"

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

	struct Renderer2DData
	{
		// Max for draw calls
		static const uint32_t MaxQuads = 20000; // 20,000
		static const uint32_t MaxVertices = MaxQuads * 4; // 80,000
		static const uint32_t MaxIndices = MaxQuads * 6; // 120,000
		static const uint32_t MaxTextureSlots = 32; // TODO: Render Capabilities

		Ref<VertexArray> QuadVertexArray;
		Ref<VertexBuffer> QuadVertexBuffer;
		Ref<Shader> QuadShader;

		Ref<VertexArray> CircleVertexArray;
		Ref<VertexBuffer> CircleVertexBuffer;
		Ref<Shader> CircleShader;

		uint32_t QuadIndexCount = 0;
		QuadVertex* QuadVertexBufferBase = nullptr;
		QuadVertex* QuadVertexBufferPtr = nullptr;

		uint32_t CircleIndexCount = 0;
		CircleVertex* CircleVertexBufferBase = nullptr;
		CircleVertex* CircleVertexBufferPtr = nullptr;

		std::array<Ref<Texture2D>, MaxTextureSlots> TextureSlots;
		uint32_t TextureSlotIndex = 0; // Default index is 1 because, index 0 is White Texture.

		static const uint8_t QuadVertexCount = 4;
		glm::vec4 QuadVertexPositions[4];
		glm::vec2* QuadTextureCoordinates;

		Renderer2D::Statistics Stats;

		struct CameraData
		{
			glm::mat4 ViewProjection;
		};
		CameraData CameraBuffer;
		Ref<UniformBuffer> CameraUniformBuffer;
	};

	static Renderer2DData sData;
	static std::future<void> _asyncShaderCreation;

	void Renderer2D::Init()
	{
		HZ_PROFILE_FUNCTION();

		// Quads
		sData.QuadVertexArray = VertexArray::Create();

		sData.QuadVertexBuffer = VertexBuffer::Create(sData.MaxVertices * sizeof(QuadVertex));
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

		// Circles
		sData.CircleVertexArray = VertexArray::Create();

		sData.CircleVertexBuffer = VertexBuffer::Create(sData.MaxVertices * sizeof(CircleVertex));
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
		sData.CircleVertexBufferBase = new CircleVertex[sData.MaxVertices];

		// -- Shader loading
#define ASYNC 1

#if ASYNC
		_asyncShaderCreation = std::async(std::launch::async, []
		{
			sData.QuadShader = Shader::Create("assets/shaders/Renderer2D_Quad.glsl");
			sData.CircleShader = Shader::Create("assets/shaders/Renderer2D_Circle.glsl");
		});
#else
		sData.QuadShader = Shader::Create("assets/shaders/Renderer2D_Quad.glsl");
		sData.CircleShader = Shader::Create("assets/shaders/Renderer2D_Circle.glsl");
#endif // ASYNC

		sData.QuadVertexPositions[0] = { -0.5f,-0.5f, 0.0f, 1.0f };
		sData.QuadVertexPositions[1] = { 0.5f,-0.5f, 0.0f, 1.0f };
		sData.QuadVertexPositions[2] = { 0.5f, 0.5f, 0.0f, 1.0f };
		sData.QuadVertexPositions[3] = { -0.5f, 0.5f, 0.0f, 1.0f };

		sData.QuadTextureCoordinates = new glm::vec2[4];

		sData.CameraUniformBuffer = UniformBuffer::Create(sizeof(Renderer2DData::CameraData), 0);
	}

	void Renderer2D::Shutdown()
	{
		HZ_PROFILE_FUNCTION();

		delete[] sData.QuadVertexBufferBase;
	}

	bool Renderer2D::BeginScene(const Camera& camera, const glm::mat4& transform)
	{
		HZ_PROFILE_FUNCTION();
		return BeginScene(camera.GetProjection() * glm::inverse(transform));
	}

	bool Renderer2D::BeginScene(const OrthographicCamera& camera)
	{
		HZ_PROFILE_FUNCTION();

		return BeginScene(camera.GetViewProjectionMatrix());
	}

	bool Renderer2D::BeginScene(const EditorCamera& camera)
	{
		HZ_PROFILE_FUNCTION();

		return BeginScene(camera.GetViewProjection());
	}

	bool Renderer2D::BeginScene(glm::mat4 viewProjection)
	{
		HZ_PROFILE_FUNCTION();

		if (sData.QuadShader == nullptr || sData.CircleShader == nullptr)
		{
			return false;
		}

		if (sData.QuadShader->IsLoadingCompleted())
		{
			sData.QuadShader->CompleteInitialization();
		}
		else
		{
			return false;
		}

		if (sData.CircleShader->IsLoadingCompleted())
		{
			sData.CircleShader->CompleteInitialization();
		}
		else
		{
			return false;
		}

		sData.CameraBuffer.ViewProjection = viewProjection;
		sData.CameraUniformBuffer->SetData(&sData.CameraBuffer, sizeof(Renderer2DData::CameraData));
		Reset();

		return true;
	}

	void Renderer2D::EndScene()
	{
		HZ_PROFILE_FUNCTION()

		Flush();
	}

	void Renderer2D::Reset()
	{
		HZ_PROFILE_FUNCTION();

		sData.QuadIndexCount = 0;
		sData.QuadVertexBufferPtr = sData.QuadVertexBufferBase;
		
		sData.CircleIndexCount = 0;
		sData.CircleVertexBufferPtr = sData.CircleVertexBufferBase;

		sData.TextureSlotIndex = 0;
	}

	void Renderer2D::Flush()
	{
		HZ_PROFILE_FUNCTION();

		if (sData.QuadIndexCount > 0)
		{
			auto dataSize = (uint32_t)((uint8_t*)sData.QuadVertexBufferPtr - (uint8_t*)sData.QuadVertexBufferBase);
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

		if (sData.CircleIndexCount > 0)
		{
			auto dataSize = (uint32_t)((uint8_t*)sData.CircleVertexBufferPtr - (uint8_t*)sData.CircleVertexBufferBase);
			sData.CircleVertexBuffer->SetData(sData.CircleVertexBufferBase, dataSize);
			
			sData.CircleShader->Bind();
			RenderCommand::DrawIndexed(sData.CircleVertexArray, sData.CircleIndexCount);
			
			sData.Stats.DrawCalls++;
		}
	}

#pragma region Primitive
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

	void Renderer2D::DrawQuad(const glm::mat4& transform, const Color& color, int entityID)
	{
		HZ_PROFILE_FUNCTION();

		UpdateQuadData(transform, color, entityID);
	}
#pragma endregion

#pragma region Texture
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

	void Renderer2D::DrawQuad(const glm::mat4& transform, const Ref<Texture2D>& texture, const glm::vec2& tilingFactor, const Color& tintColor, int entityID)
	{
		HZ_PROFILE_FUNCTION();

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

		sData.QuadTextureCoordinates[0] = { 0.0f, 0.0f };
		sData.QuadTextureCoordinates[1] = { 1.0f, 0.0f };
		sData.QuadTextureCoordinates[2] = { 1.0f, 1.0f };
		sData.QuadTextureCoordinates[3] = { 0.0f, 1.0f };

		UpdateQuadData(transform, tintColor, entityID, tilingFactor, textureIndex);
	}
#pragma endregion

#pragma region SubTexture
	void Renderer2D::DrawQuad(const glm::vec2& position, const glm::vec2& size, const Ref<SubTexture2D>& subTexture, const glm::vec2& tilingFactor, const Color& tintColor)
	{
		DrawQuad({ position.x, position.y, 0.0f }, size, subTexture, tilingFactor, tintColor);
	}

	void Renderer2D::DrawQuad(const glm::vec3& position, const glm::vec2& size, const Ref<SubTexture2D>& subTexture, const glm::vec2& tilingFactor, const Color& tintColor)
	{
		HZ_PROFILE_FUNCTION();

		glm::mat4 transform = glm::translate(_sIdentityMatrix, position)
			* glm::scale(_sIdentityMatrix, { size.x,size.y,1.0f });

		DrawQuad(transform, subTexture, tilingFactor, tintColor);
	}

	void Renderer2D::DrawRotatedQuad(const glm::vec2& position, const glm::vec2& size, float rotation, const Ref<SubTexture2D>& subTexture, const glm::vec2& tilingFactor, const Color& tintColor)
	{
		DrawRotatedQuad({ position.x, position.y, 0.0f }, size, rotation, subTexture, tilingFactor, tintColor);
	}

	void Renderer2D::DrawRotatedQuad(const glm::vec3& position, const glm::vec2& size, float rotation, const Ref<SubTexture2D>& subTexture, const glm::vec2& tilingFactor, const Color& tintColor)
	{
		HZ_PROFILE_FUNCTION();

		glm::mat4 transform = glm::translate(_sIdentityMatrix, position)
			* glm::rotate(_sIdentityMatrix, glm::radians(rotation), { 0.0f,0.0f,1.0f })
			* glm::scale(_sIdentityMatrix, { size.x,size.y,1.0f });

		DrawQuad(transform, subTexture, tilingFactor, tintColor);
	}

	void Renderer2D::DrawQuad(const glm::mat4& transform, const Ref<SubTexture2D>& subTexture, const glm::vec2& tilingFactor, const Color& tintColor)
	{
		HZ_PROFILE_FUNCTION();

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

	void Renderer2D::DrawSprite(const glm::mat4& transform, SpriteRendererComponent& spriteRenderComponent, int entityID)
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

	void Renderer2D::ResetStats()
	{
		memset(&sData.Stats, 0, sizeof(Statistics));
	}

	Renderer2D::Statistics Renderer2D::GetStats()
	{
		return sData.Stats;
	}

	void Renderer2D::LoadShader(const std::string& filePath, bool shouldRecompile)
	{
		HZ_CORE_LWARN("Reloading Shader");
		sData.QuadShader = nullptr;
		_asyncShaderCreation = std::async(std::launch::async, [](const std::string& filePath, bool shouldRecompile)
		{
			sData.QuadShader = Shader::Create(filePath, shouldRecompile);
		}, filePath, shouldRecompile);
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

		for (uint32_t i = 0; i < Renderer2DData::QuadVertexCount; i++)
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

		// TODO: Implement for circles
		/*if (sData.CircleIndexCount >= Renderer2DData::MaxIndices)
		{
			FlushAndReset();
		}*/

		for (uint32_t i = 0; i < Renderer2DData::QuadVertexCount; i++)
		{
			sData.CircleVertexBufferPtr->WorldPosition = transform * sData.QuadVertexPositions[i];
			sData.CircleVertexBufferPtr->LocalPosition = sData.QuadVertexPositions[i] * 2.0f;
			sData.CircleVertexBufferPtr->Color = color;
			sData.CircleVertexBufferPtr->Thickness = thickness;
			sData.CircleVertexBufferPtr->Fade = fade;
			sData.CircleVertexBufferPtr->EntityID = entityID;
			sData.CircleVertexBufferPtr++;
		}

		sData.CircleIndexCount += 6;
		sData.Stats.QuadCount++;
	}
}
