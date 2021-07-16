#include "hzpch.h"
#include "OpenGLContext.h"

#include <GLFW/glfw3.h>
#include <glad/glad.h>


namespace Hazel
{
	OpenGLContext::OpenGLContext(GLFWwindow* windowHandle)
		:_windowHandle(windowHandle)
	{
		HZ_CORE_ASSERT(windowHandle, "Window handle is null!");
	}

	void OpenGLContext::Init()
	{
		HZ_PROFILE_FUNCTION();

		glfwMakeContextCurrent(_windowHandle);
		auto status = gladLoadGLLoader((GLADloadproc)glfwGetProcAddress); // Glad setup/initialization
		HZ_CORE_ASSERT(status, "Failed to initialize Glad!");

#if HZ_DEBUG
		HZ_CORE_LINFO("OpenGL Info:");
		HZ_CORE_LINFO("  Vendor: {0}", glGetString(GL_VENDOR));
		HZ_CORE_LINFO("  Renderer: {0}", glGetString(GL_RENDERER));
		HZ_CORE_LINFO("  Version: {0}", glGetString(GL_VERSION));
#endif // HZ_DEBUG

		HZ_CORE_ASSERT(GLVersion.major > 4 || (GLVersion.major == 4 && GLVersion.minor >= 5), "Hazel requirest at least OpenGL version 4.5!");
	}

	void OpenGLContext::SwapBuffers()
	{
		HZ_PROFILE_FUNCTION();

		glfwSwapBuffers(_windowHandle);
	}
}
