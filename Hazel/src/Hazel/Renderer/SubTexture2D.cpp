#include "hzpch.h"
#include "SubTexture2D.h"

namespace Hazel
{
	SubTexture2D::SubTexture2D(const Ref<Texture2D>& texture, const glm::vec2& min, const glm::vec2& max)
		:_texture(texture)
	{
		_textureCoords[0] = { min.x, min.y };
		_textureCoords[1] = { max.x, min.y };
		_textureCoords[2] = { max.x, max.y };
		_textureCoords[3] = { min.x, max.y };
	}

	Ref<SubTexture2D> SubTexture2D::CreateFromCoords(const Ref<Texture2D>& texture, const glm::vec2& coords, const glm::vec2& cellSize, const glm::vec2& spriteSize)
	{
		glm::vec2 min = { coords.x * cellSize.x / static_cast<float>(texture->GetWidth()), coords.y * cellSize.y / static_cast<float>(texture->GetHeight()) };
		glm::vec2 max = { (coords.x + spriteSize.x) * cellSize.x / static_cast<float>(texture->GetWidth()),	(coords.y + spriteSize.y) * cellSize.y / static_cast<float>(texture->GetHeight()) };

		return CreateRef<SubTexture2D>(texture, min, max);
	}
}
