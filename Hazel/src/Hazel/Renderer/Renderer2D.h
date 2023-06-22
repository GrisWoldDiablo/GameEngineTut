#pragma once

#include "OrthographicCamera.h"
#include "Hazel/Core/Application.h"
#include "Hazel/Core/Color.h"
#include "Hazel/Renderer/Texture.h"
#include "Hazel/Renderer/SubTexture2D.h"
#include "Hazel/Renderer/Camera.h"
#include "Hazel/Renderer/Font.h"
#include "Hazel/Scene/Components.h"

namespace Hazel
{
	enum RendererShader
	{
		QUAD,
		CIRCLE,
		LINE,
		TEXT
	};

	constexpr const char* sRendererShaderName[] = {"Quad", "Circle", "Line", "Text"};

	constexpr const char* SHADER_PATH_QUAD = "assets/shaders/Renderer2D_Quad.glsl";
	constexpr const char* SHADER_PATH_CIRCLE = "assets/shaders/Renderer2D_Circle.glsl";
	constexpr const char* SHADER_PATH_LINE = "assets/shaders/Renderer2D_Line.glsl";
	constexpr const char* SHADER_PATH_TEXT = "assets/shaders/Renderer2D_Text.glsl";

	class Renderer2D
	{
	private:
		static constexpr glm::mat4 _sIdentityMatrix = glm::identity<glm::mat4>();

	public:
		static void Init();
		static void Shutdown();

		static bool BeginScene(const Camera& camera, const glm::mat4& transform);
		static bool BeginScene(glm::mat4 viewProjection);

		static void EndScene();
		static void Flush();
		static void Reset();

		// --- Plain Quad --- //
		static void DrawQuad(const glm::vec2& position, const glm::vec2& size, const Color& color);
		static void DrawQuad(const glm::vec3& position, const glm::vec2& size, const Color& color);
		static void DrawRotatedQuad(const glm::vec2& position, const glm::vec2& size, float rotation, const Color& color);
		static void DrawRotatedQuad(const glm::vec3& position, const glm::vec2& size, float rotation, const Color& color);
		// Final Draw
		static void DrawQuad(const glm::mat4& transform, const Color& color, int entityID = -1);
		// --- ---------- --- //

		// --- Textures --- //
		static void DrawQuad(const glm::vec2& position, const glm::vec2& size, const Ref<Texture2D>& texture, const glm::vec2& tilingFactor = glm::vec2(1.0f), const Color& tintColor = Color::White, int entityID = -1);
		static void DrawQuad(const glm::vec3& position, const glm::vec2& size, const Ref<Texture2D>& texture, const glm::vec2& tilingFactor = glm::vec2(1.0f), const Color& tintColor = Color::White, int entityID = -1);
		static void DrawRotatedQuad(const glm::vec2& position, const glm::vec2& size, float rotation, const Ref<Texture2D>& texture, const glm::vec2& tilingFactor = glm::vec2(1.0f), const Color& tintColor = Color::White, int entityID = -1);
		static void DrawRotatedQuad(const glm::vec3& position, const glm::vec2& size, float rotation, const Ref<Texture2D>& texture, const glm::vec2& tilingFactor = glm::vec2(1.0f), const Color& tintColor = Color::White, int entityID = -1);
		// Final Draw
		static void DrawQuad(const glm::mat4& transform, const Ref<Texture2D>& texture, const glm::vec2& tilingFactor = glm::vec2(1.0f), const Color& tintColor = Color::White, int entityID = -1);
		// --- -------- --- //

		// --- SubTextures --- //
		static void DrawQuad(const glm::vec2& position, const glm::vec2& size, const Ref<SubTexture2D>& subTexture, const glm::vec2& tilingFactor = glm::vec2(1.0f), const Color& tintColor = Color::White);
		static void DrawQuad(const glm::vec3& position, const glm::vec2& size, const Ref<SubTexture2D>& subTexture, const glm::vec2& tilingFactor = glm::vec2(1.0f), const Color& tintColor = Color::White);
		static void DrawRotatedQuad(const glm::vec2& position, const glm::vec2& size, float rotation, const Ref<SubTexture2D>& subTexture, const glm::vec2& tilingFactor = glm::vec2(1.0f), const Color& tintColor = Color::White);
		static void DrawRotatedQuad(const glm::vec3& position, const glm::vec2& size, float rotation, const Ref<SubTexture2D>& subTexture, const glm::vec2& tilingFactor = glm::vec2(1.0f), const Color& tintColor = Color::White);
		// Final Draw
		static void DrawQuad(const glm::mat4& transform, const Ref<SubTexture2D>& subTexture, const glm::vec2& tilingFactor = glm::vec2(1.0f), const Color& tintColor = Color::White);
		// --- ----------- --- //

		// --- Sprite --- //
		// Final Draw
		static void DrawSprite(const glm::mat4& transform, const SpriteRendererComponent& spriteRenderComponent, int entityID);
		// --- ----------- --- //

		// --- Circle --- //
		// Final Draw
		static void DrawCircle(const glm::mat4& transform, const Color& color, float thickness = 1.0f, float fade = 0.005f, int entityID = -1);
		// --- ----------- --- //

		// --- Line --- //
		// Final Draw
		static void DrawLine(const glm::vec3& positionStart, const glm::vec3& positionEnd, const Color& color, int entityID = -1);
		static void DrawRect(const glm::vec3& position, const glm::vec2& size, const Color& color, int entityID = -1);
		static void DrawRect(const glm::mat4& transform, const Color& color, int entityID = -1);

		static float GetLineWidth();
		static void SetLineWidth(float width);
		// --- ----------- --- //

		// --- Text --- //
		static void DrawString(const glm::mat4& transform, const TextComponent& textComponent, int entityID);
		// Final Draw
		static void DrawString(const glm::mat4& transform, const std::string& string, const Ref<Font>& font = Font::GetDefault(), const Color& color = Color::White, float kerning = 0.0f, float lineSpace = 0.0f, int entityID = -1);
		// --- ----------- --- //

		struct Statistics
		{
			uint32_t DrawCalls = 0;
			uint32_t QuadCount = 0;

			uint32_t GetTotalVertexCount() const { return QuadCount * 4; }
			uint32_t GetTotalIndexCount() const { return QuadCount * 6; }
		};

		static void ResetStats();
		static Statistics GetStats();

		static bool IsReady();
		static void ReloadShader(RendererShader rendererShader);
		static void LoadShadersAsync();

	private:
		static void FlushAndReset();
		static void UpdateQuadData(const glm::mat4& transform, const Color& color, int entityID = -1, const glm::vec2& tilingFactor = glm::vec2(1.0f), int textureIndex = -1);
		static void UpdateCircleData(const glm::mat4& transform, const Color& color, int entityID = -1, float thickness = 1.0f, float fade = 0.005f);

		static void DrawInvalidTexture(const glm::mat4& transform);
	};
}
