#include "hzpch.h"
#include "Renderer.h"
#include "Renderer2D.h"

#include "Platform/OpenGL/OpenGLShader.h"

namespace Hazel
{
	Scope<Renderer::SceneData> Renderer::_sSceneData = CreateScope<SceneData>();
	
	void Renderer::Init()
	{
		RenderCommand::Init();
		Renderer2D::Init();
	}

	void Renderer::OnWindowResize(uint32_t width, uint32_t height)
	{
		RenderCommand::SetViewport(0, 0, width, height);
	}

	void Renderer::BeginScene(OrthographicCamera& camera)
	{
		_sSceneData->ViewProjectionMatrix = camera.GetViewProjectionMatrix();
	}

	void Renderer::EndScene()
	{
	}

	void Renderer::Submit(const Ref<Shader>& shader, const Ref<VertexArray>& vertexArray, const glm::mat4& transform)
	{
		shader->Bind();
		std::dynamic_pointer_cast<OpenGLShader>(shader)->UploadUniformMat4("u_ViewProjection", _sSceneData->ViewProjectionMatrix);
		std::dynamic_pointer_cast<OpenGLShader>(shader)->UploadUniformMat4("u_Transform", transform);
		
		vertexArray->Bind();
		RenderCommand::DrawIndexed(vertexArray);
	}
}
