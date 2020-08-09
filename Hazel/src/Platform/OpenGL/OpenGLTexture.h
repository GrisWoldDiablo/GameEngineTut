#pragma once
#include "Hazel/Renderer/Texture.h"

namespace Hazel
{
	class OpenGLTexture2D : public Texture2D
	{
	public:
		OpenGLTexture2D(std::string path);
		~OpenGLTexture2D();

		uint32_t GetWidth() const override { return _width; }
		uint32_t GetHeight() const override { return _height; }

		void Bind(uint32_t slot = 0) const override;
	private:
		std::string _path;
		uint32_t _width;
		uint32_t _height;
		uint32_t _rendererID;
	};
}
