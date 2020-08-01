#include "hzpch.h"
#include "Buffer.h"

#include "Renderer.h"

#include "Platform/OpenGL/OpenGLBuffer.h"  

namespace Hazel
{
	VertexBuffer* VertexBuffer::Create(float* vertices, uint32_t size)
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::API::None:
			HZ_CORE_ASSERT(false, "RendererAPI::None is currently not supported.");
		case RendererAPI::API::OpenGL:
			return new OpenGLVertexBuffer(vertices, size);
		case RendererAPI::API::DirectX:
			HZ_CORE_ASSERT(false, "RendererAPI::DirectX is currently not supported.");
		case RendererAPI::API::Vulkan:
			HZ_CORE_ASSERT(false, "RendererAPI::Vulkan is currently not supported.");
		}

		HZ_CORE_ASSERT(false, "Unknow RendererAPI, VertexBuffer::Create");
		return nullptr;
	}

	IndexBuffer* IndexBuffer::Create(uint32_t* indices, uint32_t count)
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::API::None:
			HZ_CORE_ASSERT(false, "RendererAPI::None is currently not supported.");
		case RendererAPI::API::OpenGL:
			return new OpenGLIndexBuffer(indices, count);
		case RendererAPI::API::DirectX:
			HZ_CORE_ASSERT(false, "RendererAPI::DirectX is currently not supported.");
		case RendererAPI::API::Vulkan:
			HZ_CORE_ASSERT(false, "RendererAPI::Vulkan is currently not supported.");
		default: ;
		}

		HZ_CORE_ASSERT(false, "Unknow RendererAPI, IndexBuffer::Create");
		return nullptr;
	}
}
