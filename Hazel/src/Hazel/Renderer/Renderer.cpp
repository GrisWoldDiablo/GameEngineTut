#include "hzpch.h"
#include "Renderer.h"
#include "Renderer2D.h"

#include "Platform/OpenGL/OpenGLShader.h"

namespace Hazel
{
	Scope<Renderer::SceneData> Renderer::_sSceneData = CreateScope<SceneData>();

	void Renderer::Init()
	{
		HZ_PROFILE_FUNCTION();

		RenderCommand::Init();
		Renderer2D::Init();
	}

	void Renderer::Shutdown()
	{
		Renderer2D::Shutdown();
	}

	void Renderer::OnWindowResize(uint32_t width, uint32_t height)
	{
		HZ_PROFILE_FUNCTION();

		RenderCommand::SetViewport(0, 0, width, height);
	}

	void Renderer::BeginScene(OrthographicCamera& camera)
	{
		HZ_PROFILE_FUNCTION();

		_sSceneData->ViewProjectionMatrix = camera.GetViewProjectionMatrix();
	}

	void Renderer::EndScene()
	{}

	void Renderer::Submit(const Ref<Shader>& shader, const Ref<VertexArray>& vertexArray, const glm::mat4& transform)
	{
		HZ_PROFILE_FUNCTION();

		shader->Bind();
		std::dynamic_pointer_cast<OpenGLShader>(shader)->UploadUniformMat4("u_ViewProjection", _sSceneData->ViewProjectionMatrix);
		std::dynamic_pointer_cast<OpenGLShader>(shader)->UploadUniformMat4("u_Transform", transform);

		vertexArray->Bind();
		RenderCommand::DrawIndexed(vertexArray);
	}
}
