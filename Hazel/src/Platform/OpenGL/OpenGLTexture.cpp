#include "hzpch.h"
#include "OpenGLTexture.h"

#include "stb_image.h"

namespace Hazel
{
	OpenGLTexture2D::OpenGLTexture2D(uint32_t width, uint32_t height)
		:_width(width), _height(height), _internalFormat(GL_RGBA8), _dataFormat(GL_RGBA)
	{
		HZ_PROFILE_FUNCTION();

		glCreateTextures(GL_TEXTURE_2D, 1, &_rendererID);
		glTextureStorage2D(_rendererID, 1, _internalFormat, _width, _height);

		glTextureParameteri(_rendererID, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTextureParameteri(_rendererID, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

		// Tiling.
		glTextureParameteri(_rendererID, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTextureParameteri(_rendererID, GL_TEXTURE_WRAP_T, GL_REPEAT);
	}

	OpenGLTexture2D::OpenGLTexture2D(std::string path)
		:_path(std::move(path))
	{
		HZ_PROFILE_FUNCTION();

		int width, height, channels;
		stbi_set_flip_vertically_on_load(1);

		stbi_uc* data = nullptr;
		{
			HZ_PROFILE_SCOPE("stbi_load");
			data = stbi_load(_path.c_str(), &width, &height, &channels, 0);
		}
		HZ_CORE_ASSERT(data, "Failed to load image!");

		_width = width;
		_height = height;

		GLenum internalFormat = 0;
		GLenum dataFormat = 0;
		if (channels == 4)
		{
			internalFormat = GL_RGBA8;
			dataFormat = GL_RGBA;
		}
		else if (channels == 3)
		{
			internalFormat = GL_RGB8;
			dataFormat = GL_RGB;
		}

		HZ_CORE_ASSERT(internalFormat & dataFormat, "Format not supported!");

		_internalFormat = internalFormat;
		_dataFormat = dataFormat;

		glCreateTextures(GL_TEXTURE_2D, 1, &_rendererID);
		glTextureStorage2D(_rendererID, 1, _internalFormat, _width, _height);

		glTextureParameteri(_rendererID, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTextureParameteri(_rendererID, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

		// Tiling.
		glTextureParameteri(_rendererID, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTextureParameteri(_rendererID, GL_TEXTURE_WRAP_T, GL_REPEAT);

		glTextureSubImage2D(_rendererID, 0, 0, 0, _width, _height, _dataFormat, GL_UNSIGNED_BYTE, data);

		stbi_image_free(data);
	}

	OpenGLTexture2D::~OpenGLTexture2D()
	{
		HZ_PROFILE_FUNCTION();

		glDeleteTextures(1, &_rendererID);
	}

	void OpenGLTexture2D::SetData(void* data, uint32_t size)
	{
		HZ_PROFILE_FUNCTION();

		uint32_t bytesPerPixel = _dataFormat == GL_RGBA ? 4 : 3;
		HZ_ASSERT(size == _width * _height * bytesPerPixel, "Data must be entire texture!");
		glTextureSubImage2D(_rendererID, 0, 0, 0, _width, _height, _dataFormat, GL_UNSIGNED_BYTE, data);
	}

	void OpenGLTexture2D::Bind(uint32_t slot) const
	{
		HZ_PROFILE_FUNCTION();

		glBindTextureUnit(slot, _rendererID);
	}

	bool OpenGLTexture2D::Equals(const Texture& other) const
	{
		return _rendererID == ((OpenGLTexture2D&)other)._rendererID;
	}
}
