#include "hzpch.h"
#include "Shader.h"

#include "Renderer.h"
#include "Platform/OpenGL/OpenGLShader.h"

namespace Hazel
{
	Shader* Shader::Create(const std::string& vertexSrc, const std::string& fragmentSrc)
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::API::None:
			HZ_CORE_ASSERT(false, "RendererAPI::None is currently not supported.");
		case RendererAPI::API::OpenGL:
			return new OpenGLShader(vertexSrc,fragmentSrc);
		case RendererAPI::API::DirectX:
			HZ_CORE_ASSERT(false, "RendererAPI::DirectX is currently not supported.");
		case RendererAPI::API::Vulkan:
			HZ_CORE_ASSERT(false, "RendererAPI::Vulkan is currently not supported.");
		default:;
		}

		HZ_CORE_ASSERT(false, "Unknow RendererAPI, Shader::Create");
		return nullptr;
	}
}
