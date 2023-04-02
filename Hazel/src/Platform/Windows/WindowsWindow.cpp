#include "hzpch.h"
#include "WindowsWindow.h"

#include "Hazel/Core/Input.h"
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

			const auto success = glfwInit();
			HZ_CORE_ASSERT(success, "Could not initialized GLFW!");

			// Made a static function to be the callback.
			glfwSetErrorCallback(GLFWErrorCallback);
			sGLFWInitialized = true;
		}

		{
			HZ_PROFILE_SCOPE("glfwCreateWindow");

			_window = glfwCreateWindow(static_cast<int>(_data.Width), static_cast<int>(_data.Height), _data.Title.c_str(), nullptr, nullptr);
		}

		_context = CreateScope<OpenGLContext>(_window);
		_context->Init();

		glfwSetWindowUserPointer(_window, &_data);
		SetVSync(props.VSync);

		{
			HZ_PROFILE_SCOPE("glfmSetCallbacks");

			// Set GLFW callbacks
			glfwSetWindowSizeCallback(_window, [](GLFWwindow* window, int width, int height)
			{
				auto* data = static_cast<WindowData*>(glfwGetWindowUserPointer(window));
				data->Width = width;
				data->Height = height;

				WindowResizeEvent event(width, height);
				data->EventCallback(event);
			});

			glfwSetWindowCloseCallback(_window, [](GLFWwindow* window)
			{
				const auto* data = static_cast<WindowData*>(glfwGetWindowUserPointer(window));
				WindowCloseEvent event;
				data->EventCallback(event);
			});

			glfwSetKeyCallback(_window, [](GLFWwindow* window, int key, int scancode, int action, int mods)
			{
				if (!Input::IsValidKeyCodeValue(key))
				{
					return;
				}

				const auto* data = static_cast<WindowData*>(glfwGetWindowUserPointer(window));
				const auto keyCode = static_cast<KeyCode>(key);
				auto& keyStatus = Input::Get().GetKeyStatus(keyCode);

				// TODO : Add repeat event?
				switch (action)
				{
				case GLFW_PRESS:
				{
					keyStatus = Input::Status::Pressed;

					KeyPressedEvent event(keyCode);
					data->EventCallback(event);
					break;
				}
				case GLFW_RELEASE:
				{
					keyStatus = Input::Status::Up;

					KeyUpEvent event(keyCode);
					data->EventCallback(event);
					break;
				}
				default: break;
				}
			});

			glfwSetCharCallback(_window, [](GLFWwindow* window, unsigned int key)
			{
				const auto* data = static_cast<WindowData*>(glfwGetWindowUserPointer(window));
				KeyTypedEvent event(static_cast<KeyCode>(key));
				data->EventCallback(event);
			});

			glfwSetMouseButtonCallback(_window, [](GLFWwindow* window, int button, int action, int mods)
			{
				if (!Input::IsValidMouseCodeValue(button))
				{
					return;
				}
				
				const auto* data = static_cast<WindowData*>(glfwGetWindowUserPointer(window));

				const auto mouseCode = static_cast<MouseCode>(button);
				auto& mouseStatus = Input::Get().GetMouseStatus(mouseCode);

				// TODO : Add repeat event?
				switch (action)
				{
				case GLFW_PRESS:
				{
					mouseStatus = Input::Status::Pressed;

					MouseButtonPressedEvent event(mouseCode);
					data->EventCallback(event);
					break;
				}
				case GLFW_RELEASE:
				{
					mouseStatus = Input::Status::Up;

					MouseButtonUpEvent event(mouseCode);
					data->EventCallback(event);
					break;
				}
				default: break;
				}
			});

			glfwSetScrollCallback(_window, [](GLFWwindow* window, double xOffset, double yOffset)
			{
				const auto* data = static_cast<WindowData*>(glfwGetWindowUserPointer(window));

				MouseScrolledEvent event((float)xOffset, (float)yOffset);
				data->EventCallback(event);
			});

			glfwSetCursorPosCallback(_window, [](GLFWwindow* window, double xPos, double yPos)
			{
				const auto* data = static_cast<WindowData*>(glfwGetWindowUserPointer(window));

				MouseMovedEvent event((float)xPos, (float)yPos);
				data->EventCallback(event);
			});

			glfwSetWindowPosCallback(_window, [](GLFWwindow* window, int x, int y)
			{
				const auto* data = static_cast<WindowData*>(glfwGetWindowUserPointer(window));
				WindowMovedEvent event(x, y);
				data->EventCallback(event);
			});
		}
	}

	void WindowsWindow::Shutdown() const
	{
		HZ_PROFILE_FUNCTION();

		glfwDestroyWindow(_window);
		glfwTerminate();
	}

	void WindowsWindow::SetTitle(const std::string& title)
	{
		glfwSetWindowTitle(_window, title.c_str());
	}

	void WindowsWindow::OnUpdate()
	{
		HZ_PROFILE_FUNCTION();

		if (this->GetWidth() == 0 || this->GetHeight() == 0)
		{
			return;
		}

		_context->SwapBuffers();
	}

	void WindowsWindow::ProcessEvents()
	{
		glfwPollEvents();
	}

	void WindowsWindow::SetVSync(bool enable)
	{
		HZ_PROFILE_FUNCTION();

		glfwSwapInterval(enable ? 1 : 0);

		_data.VSync = enable;
	}
}
