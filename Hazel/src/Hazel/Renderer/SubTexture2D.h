#pragma once

#include <glm/glm.hpp>

#include "Texture.h"

namespace Hazel
{
	class SubTexture2D
	{
	public:
		SubTexture2D(const Ref<Texture2D>& texture, const glm::vec2& min, const glm::vec2& max);

		Ref<Texture2D> GetTexture() const { return _texture; }
		const glm::vec2* GetTexCoords() const { return _textureCoords; }

		static Ref<SubTexture2D> CreateFromCoords(const Ref<Texture2D>& texture, const glm::vec2& coords, const glm::vec2& cellSize, const glm::vec2& spriteSize = {1, 1});
	private:
		Ref<Texture2D> _texture;

		glm::vec2 _textureCoords[4];
	};
}
