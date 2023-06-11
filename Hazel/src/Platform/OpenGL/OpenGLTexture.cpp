#include "hzpch.h"
#include "OpenGLTexture.h"

#include "stb_image.h"

namespace Hazel
{
	namespace Utils
	{
		static GLenum HazelImageFormatToGLDataFormat(ImageFormat format)
		{
			switch (format)
			{
			case ImageFormat::RGB8: return GL_RGB;
			case ImageFormat::RGBA8: return GL_RGBA;
			}

			HZ_CORE_ASSERT(false, "Unsupported Format.")
			return 0;
		}

		static GLenum HazelImageFormatToGLInternalFormat(ImageFormat format)
		{
			switch (format)
			{
			case ImageFormat::RGB8: return GL_RGB8;
			case ImageFormat::RGBA8: return GL_RGBA8;
			}

			HZ_CORE_ASSERT(false, "Unsupported Format.")
			return 0;
		}
	}

	OpenGLTexture2D::OpenGLTexture2D(const TextureSpecification& specification)
		: _specification(specification), _width(_specification.Width), _height(_specification.Height)
	{
		HZ_PROFILE_FUNCTION();

		_internalFormat = Utils::HazelImageFormatToGLInternalFormat(_specification.Format);
		_dataFormat = Utils::HazelImageFormatToGLDataFormat(_specification.Format);

		glCreateTextures(GL_TEXTURE_2D, 1, &_rendererID);
		glTextureStorage2D(_rendererID, 1, _internalFormat, _width, _height);

		glTextureParameteri(_rendererID, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		_magFilter = GL_LINEAR;
		glTextureParameteri(_rendererID, GL_TEXTURE_MAG_FILTER, _magFilter);

		// Tiling.
		glTextureParameteri(_rendererID, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTextureParameteri(_rendererID, GL_TEXTURE_WRAP_T, GL_REPEAT);
	}

	OpenGLTexture2D::OpenGLTexture2D(const std::filesystem::path& path)
		: _path(path)
	{
		HZ_PROFILE_FUNCTION();

		HZ_CORE_ASSERT(std::filesystem::exists(path), fmt::format("File path {0} not found!", path.string()));

		int width, height, channels;
		stbi_set_flip_vertically_on_load(1);

		stbi_uc* data = nullptr;
		{
			HZ_PROFILE_SCOPE("stbi_load");
			data = stbi_load(_path.string().c_str(), &width, &height, &channels, 0);
		}

		const bool hasLoadingFailed = !data;
		if (hasLoadingFailed)
		{
			stbi_image_free(data);
			HZ_CORE_ASSERT(false, "Failed to load image!");
			uint8_t dataError[4] = {255, 128, 255, 255};
			data = dataError;
			width = 1;
			height = 1;
			channels = 4;
		}

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
		_magFilter = GL_LINEAR;
		glTextureParameteri(_rendererID, GL_TEXTURE_MAG_FILTER, _magFilter);

		// Tiling.
		glTextureParameteri(_rendererID, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTextureParameteri(_rendererID, GL_TEXTURE_WRAP_T, GL_REPEAT);

		glTextureSubImage2D(_rendererID, 0, 0, 0, _width, _height, _dataFormat, GL_UNSIGNED_BYTE, data);

		if (!hasLoadingFailed)
		{
			stbi_image_free(data);
		}
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
		return _rendererID == dynamic_cast<const OpenGLTexture2D&>(other)._rendererID;
	}

	void OpenGLTexture2D::SetMagFilter(uint32_t magFilter)
	{
		_magFilter = magFilter;
		glTextureParameteri(_rendererID, GL_TEXTURE_MAG_FILTER, _magFilter);
	}

	void OpenGLTexture2D::ToggleMagFilter(uint32_t magFilter)
	{
		_magFilter = magFilter == GL_LINEAR ? GL_NEAREST : GL_LINEAR;
		glTextureParameteri(_rendererID, GL_TEXTURE_MAG_FILTER, _magFilter);
	}
}
