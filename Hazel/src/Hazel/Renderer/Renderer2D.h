#pragma once

#include "OrthographicCamera.h"
#include "Hazel\Renderer\Texture.h"
#include "Hazel\Core\Color.h"

namespace Hazel
{
	class Renderer2D
	{
	public:
		static void Init();
		static void Shutdown();

		static void BeginScene(OrthographicCamera& camera);
		static void EndScene();

		// Primitives
		static void DrawQuad(const glm::vec2& position, const glm::vec2& size, const Color& color);
		static void DrawQuad(const glm::vec3& position, const glm::vec2& size, const Color& color);
		static void DrawQuad(const glm::vec2& position, const glm::vec2& size, float rotation, const Color& color);
		static void DrawQuad(const glm::vec3& position, const glm::vec2& size, float rotation, const Color& color);
		static void DrawQuad(const glm::vec2& position, const glm::vec2& size, const Ref<Texture2D> texture,
			const glm::vec2& tilingFactor = glm::vec2(1.0f), const Color& color = Color(1.0f));
		static void DrawQuad(const glm::vec3& position, const glm::vec2& size, const Ref<Texture2D> texture,
			const glm::vec2& tilingFactor = glm::vec2(1.0f), const Color& color = Color(1.0f));
		static void DrawQuad(const glm::vec2& position, const glm::vec2& size, float rotation, const Ref<Texture2D> texture,
			const glm::vec2& tilingFactor = glm::vec2(1.0f), const Color& color = Color(1.0f));
		static void DrawQuad(const glm::vec3& position, const glm::vec2& size, float rotation, const Ref<Texture2D> texture,
			const glm::vec2& tilingFactor = glm::vec2(1.0f), const Color& color = Color(1.0f));
	};
}
