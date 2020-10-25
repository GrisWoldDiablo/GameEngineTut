#include "hzpch.h"
#include "Framebuffer.h"
#include "Renderer.h"

#include "Platform/OpenGL/OpenGLFramebuffer.h"

namespace Hazel
{
	
	Ref<Framebuffer> Framebuffer::Create(const FramebufferSpecification& spec)
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::API::None:
			HZ_CORE_ASSERT(false, "RendererAPI::None is currently not supported.") { return nullptr; }
		case RendererAPI::API::OpenGL:
			return CreateRef<OpenGLFramebuffer>(spec);
		case RendererAPI::API::DirectX:
			HZ_CORE_ASSERT(false, "RendererAPI::DirectX is currently not supported.") { return nullptr; }
		case RendererAPI::API::Vulkan:
			HZ_CORE_ASSERT(false, "RendererAPI::Vulkan is currently not supported.") { return nullptr; }
		default:
			HZ_CORE_ASSERT(false, "Unknow RendererAPI, VertexBuffer::Create") { return nullptr; }
		}
	}
}
