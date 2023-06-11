#pragma once
#include "Hazel/Renderer/Texture.h"

#include <glad/glad.h>

namespace Hazel
{
	class OpenGLTexture2D : public Texture2D
	{
	public:
		OpenGLTexture2D(const TextureSpecification& specification);
		OpenGLTexture2D(const std::filesystem::path& path);
		virtual ~OpenGLTexture2D() override;

		const TextureSpecification& GetSpecification() const override { return _specification; }

		uint32_t GetWidth() const override { return _width; }
		uint32_t GetHeight() const override { return _height; }
		uint32_t GetRendererID() const override { return _rendererID; }
		void* GetRawID() const override { return reinterpret_cast<void*>(static_cast<intptr_t>(_rendererID)); }
		const std::filesystem::path& GetPath() const override { return _path; }
		uint32_t GetMagFilter() const override { return _magFilter; }
		bool IsMagFilterLinear() const override { return _magFilter == GL_LINEAR; }

		void SetData(void* data, uint32_t size) override;
		void SetMagFilter(uint32_t magFilter) override;
		void ToggleMagFilter(uint32_t magFilter) override;

		void Bind(uint32_t slot = 0) const override;
		bool Equals(const Texture& other) const override;

		bool operator ==(const Texture& other) const override
		{
			return _rendererID == dynamic_cast<const OpenGLTexture2D&>(other)._rendererID;
		}

	private:
		TextureSpecification _specification;

		std::filesystem::path _path;
		uint32_t _width;
		uint32_t _height;
		uint32_t _rendererID;
		GLenum _internalFormat;
		GLenum _dataFormat;
		uint32_t _magFilter;
	};
}
