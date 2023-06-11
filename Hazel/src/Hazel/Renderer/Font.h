#pragma once

#include "Texture.h"

namespace Hazel
{
	struct MSDFData;

	class Font
	{
	public:
		Font(const std::filesystem::path& filepath);
		~Font();

		MSDFData* GetMSDFData() const { return _data; }
		Ref<Texture2D> GetAtlasTexture() const { return _atlasTexture; }

	private:
		MSDFData* _data;
		Ref<Texture2D> _atlasTexture;
	};
}
