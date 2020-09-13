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

		// #TODO: This will allow for rendering using z axis but disables the blending.
		//		  Re enable blending eventually.
		EnableDepthTest();
	}

	void OpenGLRendererAPI::EnableDepthTest()
	{
		glEnable(GL_DEPTH_TEST);
		glDepthMask(GL_TRUE);
	}

	void OpenGLRendererAPI::ReadOnlyDepthTest()
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

	void OpenGLRendererAPI::DrawIndexed(const Ref<VertexArray>& vertexArray, uint32_t count)
	{
		uint32_t indexCount = count ?  count : vertexArray->GetIndexBuffer()->GetCount() ;
		glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, nullptr);
	}

	float OpenGLRendererAPI::GetTime()
	{
		return (float)glfwGetTime();
	}
}
