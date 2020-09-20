#pragma once

#include "OrthographicCamera.h"
#include "Hazel\Renderer\Texture.h"
#include "Hazel\Core\Color.h"

#include <glm/gtc/matrix_transform.hpp>

namespace Hazel
{
	class Renderer2D
	{
	private:
		static constexpr glm::mat4 _sIdentityMatrix = glm::identity<glm::mat4>();

	public:
		static void Init();
		static void Shutdown();

		static void BeginScene(OrthographicCamera& camera, bool isGrayscale = false);
		static void EndScene();
		static void Flush();
		static void Reset();

		// Primitives
		static void DrawQuad(const glm::vec2& position, const glm::vec2& size, const Color& color);
		static void DrawQuad(const glm::vec3& position, const glm::vec2& size, const Color& color);
		static void DrawQuad(const glm::vec2& position, const glm::vec2& size, const Ref<Texture2D>& texture, const glm::vec2& tilingFactor = glm::vec2(1.0f), const Color& tintColor = Color(1.0f));
		static void DrawQuad(const glm::vec3& position, const glm::vec2& size, const Ref<Texture2D>& texture, const glm::vec2& tilingFactor = glm::vec2(1.0f), const Color& tintColor = Color(1.0f));
		
		static void DrawQuad(const glm::mat4& transform, const Color& color);
		static void DrawQuad(const glm::mat4& transform, const Ref<Texture2D>& texture, const glm::vec2& tilingFactor = glm::vec2(1.0f), const Color& tintColor = Color(1.0f));

		static void DrawRotatedQuad(const glm::vec2& position, const glm::vec2& size, float rotation, const Color& color);
		static void DrawRotatedQuad(const glm::vec3& position, const glm::vec2& size, float rotation, const Color& color);
		static void DrawRotatedQuad(const glm::vec2& position, const glm::vec2& size, float rotation, const Ref<Texture2D>& texture, const glm::vec2& tilingFactor = glm::vec2(1.0f), const Color& tintColor = Color(1.0f));
		static void DrawRotatedQuad(const glm::vec3& position, const glm::vec2& size, float rotation, const Ref<Texture2D>& texture, const glm::vec2& tilingFactor = glm::vec2(1.0f), const Color& tintColor = Color(1.0f));

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
		static void UpdateData(const glm::mat4& transform, const Hazel::Color& color, const glm::vec2& tilingFactor = glm::vec2(1.0f), float textureIndex = 0.0f);
	};
}
