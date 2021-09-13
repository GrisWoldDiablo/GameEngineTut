#pragma once
#include "Hazel/Renderer/Texture.h"

#include <glad/glad.h>

namespace Hazel
{
	class OpenGLTexture2D : public Texture2D
	{
	public:
		OpenGLTexture2D(uint32_t width, uint32_t height);
		OpenGLTexture2D(std::string path);
		~OpenGLTexture2D();

		uint32_t GetWidth() const override { return _width; }
		uint32_t GetHeight() const override { return _height; }
		uint32_t GetRendererID() const override { return _rendererID; }
		std::string GetPath() const override { return _path; }
		uint32_t GetMagFilter() const override { return _magFilter; }

		void SetData(void* data, uint32_t size) override;
		void SetMagFilter(uint32_t magFilter) override;
		void ToggleMagFilter(uint32_t magFilter) override;

		void Bind(uint32_t slot = 0) const override;
		bool Equals(const Texture& other) const override;

		bool operator ==(const Texture& other) const override
		{
			return _rendererID == ((OpenGLTexture2D&)other)._rendererID;
		}

	private:
		std::string _path;
		uint32_t _width;
		uint32_t _height;
		uint32_t _rendererID;
		GLenum _internalFormat;
		GLenum _dataFormat;
		uint32_t _magFilter;
	};
}
