#include "hzpch.h"
#include "Platform.h"

#include "Hazel/Renderer/RenderCommand.h"
#include "Hazel/Renderer/Renderer.h"
#include "OpenGL/OpenGLRendererAPI.h"

namespace Hazel
{
	static Ref<RendererAPI> CreateRendererAPI();

	//--- Initialize the current platform. ---//
	// Currently using OpenGL since its only one supported.
	RendererAPI::API RendererAPI::_sAPI = API::OpenGL;
	auto renderAPI = CreateRendererAPI();
	Ref<RendererAPI> RenderCommand::_sRendererAPI = renderAPI;
	Scope<Platform> Platform::_sInstance = CreateScope<Platform>(renderAPI);
	//----------------------------------------//

	static Ref<RendererAPI> CreateRendererAPI()
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::API::None:
			HZ_CORE_ASSERT(false, "RendererAPI::None is currently not supported.") { return nullptr; }
		case RendererAPI::API::OpenGL:
			return CreateRef<OpenGLRendererAPI>();
		case RendererAPI::API::DirectX:
			HZ_CORE_ASSERT(false, "RendererAPI::DirectX is currently not supported.") { return nullptr; }
		case RendererAPI::API::Vulkan:
			HZ_CORE_ASSERT(false, "RendererAPI::Vulkan is currently not supported.") { return nullptr; }
		default:
			HZ_CORE_ASSERT(false, "Unknow RendererAPI, VertexBuffer::Create") { return nullptr; }
		}
	}
}
