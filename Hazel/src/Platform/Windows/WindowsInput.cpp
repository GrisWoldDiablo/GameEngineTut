#include "hzpch.h"
#include "WindowsInput.h"

#include "Hazel/Core/Application.h"
#include <GLFW/glfw3.h>

namespace Hazel
{

	Scope<Input> Input::_sInstance = CreateScope<WindowsInput>();
	
	bool WindowsInput::IsKeyPressedImpl(KeyCode key)
	{
		HZ_PROFILE_FUNCTION();

		auto* window = static_cast<GLFWwindow*>(Application::Get().GetWindow().GetNativeWindow());
		auto state = glfwGetKey(window, static_cast<int32_t>(key));

		return state == GLFW_PRESS || state == GLFW_REPEAT;
	}

	bool WindowsInput::IsMouseButtonPressedImpl(MouseCode button)
	{
		HZ_PROFILE_FUNCTION();

		auto* window = static_cast<GLFWwindow*>(Application::Get().GetWindow().GetNativeWindow());
		auto state = glfwGetMouseButton(window, static_cast<int32_t>(button));

		return state == GLFW_PRESS;
	}

	std::pair<float, float> WindowsInput::GetMousePositionImpl()
	{
		HZ_PROFILE_FUNCTION();

		auto* window = static_cast<GLFWwindow*>(Application::Get().GetWindow().GetNativeWindow());
		double xPos, yPos;
		glfwGetCursorPos(window, &xPos, &yPos);

		return {(float)xPos, (float)yPos};
	}

	float WindowsInput::GetMouseXImpl()
	{
		HZ_PROFILE_FUNCTION();

		auto [xPos, yPos] = GetMousePositionImpl();

		return (float)xPos;
	}

	float WindowsInput::GetMouseYImpl()
	{
		HZ_PROFILE_FUNCTION();

		auto [xPos, yPos] = GetMousePositionImpl();

		return (float)yPos;
	}
}
