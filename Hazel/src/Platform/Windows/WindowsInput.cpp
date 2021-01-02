#include "hzpch.h"
#include "Hazel/Core/Input.h"

#include "Hazel/Core/Application.h"
#include <GLFW/glfw3.h>

namespace Hazel
{
	bool Input::IsKeyPressed(KeyCode key)
	{
		HZ_PROFILE_FUNCTION();

		auto* window = static_cast<GLFWwindow*>(Application::Get().GetWindow().GetNativeWindow());
		auto state = glfwGetKey(window, static_cast<int32_t>(key));
		
		return state == GLFW_PRESS;
	}

	bool Input::IsMouseButtonPressed(MouseCode button)
	{
		HZ_PROFILE_FUNCTION();

		auto* window = static_cast<GLFWwindow*>(Application::Get().GetWindow().GetNativeWindow());
		auto state = glfwGetMouseButton(window, static_cast<int32_t>(button));

		return state == GLFW_PRESS;
	}

	std::pair<float, float> Input::GetMousePosition()
	{
		HZ_PROFILE_FUNCTION();

		auto* window = static_cast<GLFWwindow*>(Application::Get().GetWindow().GetNativeWindow());
		double xPos, yPos;
		glfwGetCursorPos(window, &xPos, &yPos);

		return { (float)xPos, (float)yPos };
	}

	float Input::GetMouseX()
	{
		HZ_PROFILE_FUNCTION();

		auto [xPos, yPos] = GetMousePosition();

		return (float)xPos;
	}

	float Input::GetMouseY()
	{
		HZ_PROFILE_FUNCTION();

		auto [xPos, yPos] = GetMousePosition();

		return (float)yPos;
	}
}
