#include "hzpch.h"
#include "Texture.h"

#include "Renderer.h"
#include "Platform/OpenGL/OpenGLTexture.h"

namespace Hazel
{
	// Assignment in Hazel::Application constructor, since the renderer need to be initialized.
	Ref<Texture2D> Texture2D::ErrorTexture = nullptr;

	Ref<Texture2D> Texture2D::Create(uint32_t width, uint32_t height)
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::API::None:
			HZ_CORE_ASSERT(false, "RendererAPI::None is currently not supported."); return nullptr;
		case RendererAPI::API::OpenGL:
			return CreateRef<OpenGLTexture2D>(width, height);
		case RendererAPI::API::DirectX:
			HZ_CORE_ASSERT(false, "RendererAPI::DirectX is currently not supported."); return nullptr;
		case RendererAPI::API::Vulkan:
			HZ_CORE_ASSERT(false, "RendererAPI::Vulkan is currently not supported."); return nullptr;
		default:
			HZ_CORE_ASSERT(false, "Unknown RendererAPI, Texture2D::Create"); return nullptr;
		}
	}

	Ref<Texture2D> Texture2D::Create(const std::filesystem::path& path)
	{

		switch (Renderer::GetAPI())
		{
		case RendererAPI::API::None:
			HZ_CORE_ASSERT(false, "RendererAPI::None is currently not supported."); return nullptr;
		case RendererAPI::API::OpenGL:
			return CreateRef<OpenGLTexture2D>(path);
		case RendererAPI::API::DirectX:
			HZ_CORE_ASSERT(false, "RendererAPI::DirectX is currently not supported."); return nullptr;
		case RendererAPI::API::Vulkan:
			HZ_CORE_ASSERT(false, "RendererAPI::Vulkan is currently not supported."); return nullptr;
		default:
			HZ_CORE_ASSERT(false, "Unknown RendererAPI, Texture2D::Create"); return nullptr;
		}
	}
}
