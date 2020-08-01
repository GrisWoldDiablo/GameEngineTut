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

	auto* renderAPI = Renderer::GetAPI() == RendererAPI::API::OpenGL ? new OpenGLRendererAPI : nullptr;
	RendererAPI* RenderCommand::_sRendererAPI = renderAPI;

	Platform* Platform::_sInstance = new Platform(renderAPI);
}
