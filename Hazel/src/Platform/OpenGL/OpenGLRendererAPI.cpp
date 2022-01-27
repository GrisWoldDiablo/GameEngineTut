#include "hzpch.h"
#include "OpenGLRendererAPI.h"

#include <glad/glad.h>

#include "GLFW/glfw3.h"

namespace Hazel
{
	void OpenGLRendererAPI::Init()
	{
		HZ_PROFILE_FUNCTION();

		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glEnable(GL_DEPTH_TEST);

		// Disable for now, preferring none smooth lines.
		//glEnable(GL_LINE_SMOOTH);
	}

	void OpenGLRendererAPI::SetDepthMaskReadWrite()
	{
		glDepthMask(GL_TRUE);
	}

	void OpenGLRendererAPI::SetDepthMaskReadOnly()
	{
		glDepthMask(GL_FALSE);
	}

	void OpenGLRendererAPI::SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height)
	{
		glViewport(x, y, width, height);
	}

	void OpenGLRendererAPI::SetClearColor(const Color& color)
	{
		glClearColor(color.r, color.g, color.b, color.a);
	}

	void OpenGLRendererAPI::Clear()
	{
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}

	void OpenGLRendererAPI::DrawIndexed(const Ref<VertexArray>& vertexArray, uint32_t indexCount)
	{
		vertexArray->Bind();
		indexCount = indexCount ? indexCount : vertexArray->GetIndexBuffer()->GetCount();
		glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, nullptr);
	}

	void OpenGLRendererAPI::DrawLines(const Ref<VertexArray>& vertexArray, uint32_t vertexCount)
	{
		vertexArray->Bind();
		glDrawArrays(GL_LINES, 0, vertexCount);
	}

	float OpenGLRendererAPI::GetTime()
	{
		return (float)glfwGetTime();
	}

	void OpenGLRendererAPI::SetLineWidth(float width)
	{
		glLineWidth(width);
	}
}
