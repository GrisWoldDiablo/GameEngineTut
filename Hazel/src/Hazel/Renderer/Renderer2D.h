#pragma once

#include "OrthographicCamera.h"
#include "Hazel\Renderer\Texture.h"

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
		static void DrawQuad(const glm::vec2& position, const glm::vec2& size, glm::vec4 color);
		static void DrawQuad(const glm::vec3& position, const glm::vec2& size, glm::vec4 color);
		static void DrawQuad(const glm::vec2& position, const glm::vec2& size, const Ref<Texture2D> texture, 
			const glm::vec2& tilingFactor = glm::vec2(1.0f), const glm::vec4& color = glm::vec4(1.0f));
		static void DrawQuad(const glm::vec3& position, const glm::vec2& size, const Ref<Texture2D> texture, 
			const glm::vec2& tilingFactor = glm::vec2(1.0f), const glm::vec4& color = glm::vec4(1.0f));
	};
}
