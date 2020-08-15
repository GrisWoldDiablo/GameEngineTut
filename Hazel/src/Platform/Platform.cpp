#include "hzpch.h"
#include "Platform.h"

#include "Hazel/Renderer/RenderCommand.h"
#include "Hazel/Renderer/Renderer.h"
#include "OpenGL/OpenGLRendererAPI.h"

namespace Hazel
{
	// Initialized the current platform.
	// Currently using OpenGL since its only one supported.
	RendererAPI::API RendererAPI::_sAPI = API::OpenGL;

	Ref<RendererAPI> renderAPI = Renderer::GetAPI() == RendererAPI::API::OpenGL ? CreateRef<OpenGLRendererAPI>() : nullptr;
	Ref<RendererAPI> RenderCommand::_sRendererAPI = renderAPI;

	Scope<Platform> Platform::_sInstance = CreateScope<Platform>(renderAPI);
}
