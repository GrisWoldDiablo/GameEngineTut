#include "hzpch.h"
#include "WindowsWindow.h"

#include "Hazel/Events/ApplicationEvent.h"
#include "Hazel/Events/MouseEvent.h"
#include "Hazel/Events/KeyEvent.h"

#include "Platform/OpenGL/OpenGLContext.h"

namespace Hazel
{
	static bool sGLFWInitialized = false;

	// The callback function to errors from GLFW.
	static void GLFWErrorCallback(int errorCode, const char* description)
	{
		HZ_CORE_LERROR("GLFW Error ({0}) : {1}", errorCode, description);
	}

	Window* Window::Create(const WindowProps& props)
	{
		HZ_PROFILE_FUNCTION();

		return new WindowsWindow(props);
	}

	WindowsWindow::WindowsWindow(const WindowProps& props)
	{
		HZ_PROFILE_FUNCTION();

		Init(props);
	}

	WindowsWindow::~WindowsWindow()
	{
		HZ_PROFILE_FUNCTION();

		Shutdown();
	}

	void WindowsWindow::Init(const WindowProps& props)
	{
		HZ_PROFILE_FUNCTION();

		_data.Title = props.Title;
		_data.Width = props.Width;
		_data.Height = props.Height;

		HZ_CORE_LINFO("Creating window : {0}, ({1} x {2})", props.Title, props.Width, props.Height);

		if (!sGLFWInitialized)
		{
			HZ_PROFILE_SCOPE("glfwInit");

			auto success = glfwInit();
			HZ_CORE_ASSERT(success, "Could not initialized GLFW!")

			// Made a static function to be the callback.
			glfwSetErrorCallback(GLFWErrorCallback);
			sGLFWInitialized = true;
		}

		{
			HZ_PROFILE_SCOPE("glfwCreateWindow");

			_window = glfwCreateWindow((int)_data.Width, (int)_data.Height, _data.Title.c_str(), nullptr, nullptr);
		}


		_context = CreateScope<OpenGLContext>(_window);
		_context->Init();

		glfwSetWindowUserPointer(_window, &_data);
		SetVSync(true);

		{
			HZ_PROFILE_SCOPE("glfmSetCallbacks");

			// Set GLFW callbacks
			glfwSetWindowSizeCallback(_window, [](GLFWwindow* window, int width, int height)
				{
					auto* data = (WindowData*)glfwGetWindowUserPointer(window);
					data->Width = width;
					data->Height = height;

					WindowResizeEvent event(width, height);
					data->EventCallback(event);
				});

			glfwSetWindowCloseCallback(_window, [](GLFWwindow* window)
				{
					auto* data = (WindowData*)glfwGetWindowUserPointer(window);
					WindowCloseEvent event;
					data->EventCallback(event);
				});

			glfwSetKeyCallback(_window, [](GLFWwindow* window, int key, int scancode, int action, int mods)
				{
					auto* data = (WindowData*)glfwGetWindowUserPointer(window);
					switch (action)
					{
					case GLFW_PRESS:
					{
						KeyPressedEvent event(key, 0);
						data->EventCallback(event);
						break;
					}
					case GLFW_RELEASE:
					{
						KeyReleasedEvent event(key);
						data->EventCallback(event);
						break;
					}
					case GLFW_REPEAT:
					{
						KeyPressedEvent event(key, 1);
						data->EventCallback(event);
						break;
					}
					}
				});

			glfwSetCharCallback(_window, [](GLFWwindow* window, unsigned int key)
				{
					auto* data = (WindowData*)glfwGetWindowUserPointer(window);
					KeyTypedEvent event(key);
					data->EventCallback(event);
				});

			glfwSetMouseButtonCallback(_window, [](GLFWwindow* window, int button, int action, int mods)
				{
					auto* data = (WindowData*)glfwGetWindowUserPointer(window);

					switch (action)
					{
					case GLFW_PRESS:
					{
						MouseButtonPressedEvent event(button);
						data->EventCallback(event);
						break;
					}
					case GLFW_RELEASE:
					{
						MouseButtonReleasedEvent event(button);
						data->EventCallback(event);
						break;
					}
					}
				});

			glfwSetScrollCallback(_window, [](GLFWwindow* window, double xOffset, double yOffset)
				{
					auto* data = (WindowData*)glfwGetWindowUserPointer(window);

					MouseScrolledEvent event((float)xOffset, (float)yOffset);
					data->EventCallback(event);
				});

			glfwSetCursorPosCallback(_window, [](GLFWwindow* window, double xPos, double yPos)
				{
					auto* data = (WindowData*)glfwGetWindowUserPointer(window);

					MouseMovedEvent event((float)xPos, (float)yPos);
					data->EventCallback(event);
				});

			glfwSetWindowPosCallback(_window, [](GLFWwindow* window, int x, int y)
				{
					auto* data = (WindowData*)glfwGetWindowUserPointer(window);
					WindowMovedEvent event(x, y);
					data->EventCallback(event);
				});
		}
	}

	void WindowsWindow::Shutdown()
	{
		HZ_PROFILE_FUNCTION();

		glfwDestroyWindow(_window);
		glfwTerminate();
	}

	void WindowsWindow::OnUpdate()
	{
		HZ_PROFILE_FUNCTION();

		glfwPollEvents();
		// #TODO : Keep this?
		if (this->GetWidth() == 0 || this->GetHeight() == 0)
		{
			return;
		}
		_context->SwapBuffers();
	}

	void WindowsWindow::SetVSync(bool enable)
	{
		HZ_PROFILE_FUNCTION();

		if (enable)
		{
			glfwSwapInterval(1);
		}
		else
		{
			glfwSwapInterval(0);
		}
		_data.VSync = enable;
	}

	bool WindowsWindow::IsVSync() const
	{
		HZ_PROFILE_FUNCTION();

		return _data.VSync;
	}
}
