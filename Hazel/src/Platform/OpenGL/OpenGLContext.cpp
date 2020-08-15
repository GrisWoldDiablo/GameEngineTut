#include "hzpch.h"
#include "OpenGLContext.h"

#include <GLFW/glfw3.h>
#include <glad/glad.h>


namespace Hazel
{
	OpenGLContext::OpenGLContext(GLFWwindow* windowHandle)
		:_windowHandle(windowHandle)
	{
		HZ_CORE_ASSERT(windowHandle, "Window handle is null!")
	}

	void OpenGLContext::Init()
	{
		glfwMakeContextCurrent(_windowHandle);
		auto status = gladLoadGLLoader((GLADloadproc)glfwGetProcAddress); // Glad setup/initialization
		HZ_CORE_ASSERT(status, "Failed to initialize Glad!");
		
		HZ_CORE_LINFO("OpenGL Info:");
		HZ_CORE_LINFO("  Vendor: {0}", glGetString(GL_VENDOR));
		HZ_CORE_LINFO("  Renderer: {0}", glGetString(GL_RENDERER));
		HZ_CORE_LINFO("  Version: {0}", glGetString(GL_VERSION));

#ifdef HZ_ENABLE_ASSERTS
		int versionMajor;
		int versionMinor;
		glGetIntegerv(GL_MAJOR_VERSION, &versionMajor);
		glGetIntegerv(GL_MINOR_VERSION, &versionMinor);

		HZ_CORE_ASSERT(versionMajor > 4 || (versionMajor == 4 && versionMinor >= 5), "Hazel requirest at least OpenGL version 4.5!");
#endif // HZ_ENABLE_ASSERTS

	}

	void OpenGLContext::SwapBuffers()
	{
		glfwSwapBuffers(_windowHandle);
	}
}
