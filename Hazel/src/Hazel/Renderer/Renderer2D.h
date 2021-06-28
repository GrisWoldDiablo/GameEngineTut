#pragma once

#include "OrthographicCamera.h"
#include "Hazel/Renderer/Texture.h"
#include "Hazel/Renderer/SubTexture2D.h"
#include "Hazel/Renderer/Camera.h"
#include "Hazel/Core/Color.h"

#include <glm/gtc/matrix_transform.hpp>

#include "Hazel/Renderer/EditorCamera.h"

namespace Hazel
{
	class Renderer2D
	{
	private:
		static constexpr glm::mat4 _sIdentityMatrix = glm::identity<glm::mat4>();

	public:

		static void Init();
		static void Shutdown();

		static void BeginScene(const Camera& camera, const glm::mat4& transform);
		static void BeginScene(const EditorCamera& camera, bool isGrayscale = false); // TODO to remove
		static void BeginScene(const OrthographicCamera& camera, bool isGrayscale = false); // TODO to remove
		static void EndScene();
		static void Flush();
		static void Reset();

		// --- Primitives --- //
		static void DrawQuad(const glm::vec2& position, const glm::vec2& size, const Color& color);
		static void DrawQuad(const glm::vec3& position, const glm::vec2& size, const Color& color);
		static void DrawRotatedQuad(const glm::vec2& position, const glm::vec2& size, float rotation, const Color& color);
		static void DrawRotatedQuad(const glm::vec3& position, const glm::vec2& size, float rotation, const Color& color);
		// Final Draw
		static void DrawQuad(const glm::mat4& transform, const Color& color);
		// --- ---------- --- //

		// --- Textures --- //
		static void DrawQuad(const glm::vec2& position, const glm::vec2& size, const Ref<Texture2D>& texture, const glm::vec2& tilingFactor = glm::vec2(1.0f), const Color& tintColor = Color::White);
		static void DrawQuad(const glm::vec3& position, const glm::vec2& size, const Ref<Texture2D>& texture, const glm::vec2& tilingFactor = glm::vec2(1.0f), const Color& tintColor = Color::White);
		static void DrawRotatedQuad(const glm::vec2& position, const glm::vec2& size, float rotation, const Ref<Texture2D>& texture, const glm::vec2& tilingFactor = glm::vec2(1.0f), const Color& tintColor = Color::White);
		static void DrawRotatedQuad(const glm::vec3& position, const glm::vec2& size, float rotation, const Ref<Texture2D>& texture, const glm::vec2& tilingFactor = glm::vec2(1.0f), const Color& tintColor = Color::White);
		// Final Draw
		static void DrawQuad(const glm::mat4& transform, const Ref<Texture2D>& texture, const glm::vec2& tilingFactor = glm::vec2(1.0f), const Color& tintColor = Color::White);
		// --- -------- --- //

		// --- SubTextures --- //
		static void DrawQuad(const glm::vec2& position, const glm::vec2& size, const Ref<SubTexture2D>& subTexture, const glm::vec2& tilingFactor = glm::vec2(1.0f), const Color& tintColor = Color::White);
		static void DrawQuad(const glm::vec3& position, const glm::vec2& size, const Ref<SubTexture2D>& subTexture, const glm::vec2& tilingFactor = glm::vec2(1.0f), const Color& tintColor = Color::White);
		static void DrawRotatedQuad(const glm::vec2& position, const glm::vec2& size, float rotation, const Ref<SubTexture2D>& subTexture, const glm::vec2& tilingFactor = glm::vec2(1.0f), const Color& tintColor = Color::White);
		static void DrawRotatedQuad(const glm::vec3& position, const glm::vec2& size, float rotation, const Ref<SubTexture2D>& subTexture, const glm::vec2& tilingFactor = glm::vec2(1.0f), const Color& tintColor = Color::White);
		// Final Draw
		static void DrawQuad(const glm::mat4& transform, const Ref<SubTexture2D>& subTexture, const glm::vec2& tilingFactor = glm::vec2(1.0f), const Color& tintColor = Color::White);
		// --- ----------- --- //
		
		struct Statistics
		{
			uint32_t DrawCalls = 0;
			uint32_t QuadCount = 0;

			uint32_t GetTotalVertexCount() { return QuadCount * 4; }
			uint32_t GetTotalIndexCount() { return QuadCount * 6; }
		};

		static void ResetStats();
		static Statistics GetStats();

	private:
		static void FlushAndReset();
		static void UpdateData(const glm::mat4& transform, const Color& color, const glm::vec2& tilingFactor = glm::vec2(1.0f), float textureIndex = 0.0f);
	};
}
