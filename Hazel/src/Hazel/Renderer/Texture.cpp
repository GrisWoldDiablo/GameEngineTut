#include "hzpch.h"
#include "Texture.h"

#include "Renderer.h"
#include "Platform/OpenGL/OpenGLTexture.h"

namespace Hazel
{
	Ref<Texture2D> Texture2D::Create(std::string path)
	{

		switch (Renderer::GetAPI())
		{
		case RendererAPI::API::None:
			HZ_CORE_ASSERT(false, "RendererAPI::None is currently not supported.");
		case RendererAPI::API::OpenGL:
			return CreateRef<OpenGLTexture2D>(path);
		case RendererAPI::API::DirectX:
			HZ_CORE_ASSERT(false, "RendererAPI::DirectX is currently not supported.");
		case RendererAPI::API::Vulkan:
			HZ_CORE_ASSERT(false, "RendererAPI::Vulkan is currently not supported.");
		default:;
		}

		HZ_CORE_ASSERT(false, "Unknow RendererAPI, Texture2D::Create");
		return nullptr;
	}
}
