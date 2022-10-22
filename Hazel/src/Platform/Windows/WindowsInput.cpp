#include "hzpch.h"
#include "Hazel/Core/Input.h"

#include "Hazel/Core/Application.h"
#include <GLFW/glfw3.h>

namespace Hazel
{
	Input Input::_sInstance = Input();

	Input& Input::Get()
	{
		return _sInstance;
	}

	glm::vec2 Input::GetMousePosition()
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

		return GetMousePosition().x;
	}

	float Input::GetMouseY()
	{
		HZ_PROFILE_FUNCTION();

		return GetMousePosition().y;
	}

	void Input::UpdateDownStatus()
	{
		for (auto& keyStatus : _keysStatus)
		{
			if (keyStatus == Status::Pressed)
			{
				keyStatus = Status::Down;
			}
		}

		for (auto& buttonStatus : _buttonsStatus)
		{
			if (buttonStatus == Status::Pressed)
			{
				buttonStatus = Status::Down;
			}
		}
	}

	void Input::UpdateUpStatus()
	{
		for (auto& keyStatus : _keysStatus)
		{
			if (keyStatus == Status::Up)
			{
				keyStatus = Status::None;
			}
		}

		for (auto& buttonStatus : _buttonsStatus)
		{
			if (buttonStatus == Status::Up)
			{
				buttonStatus = Status::None;
			}
		}
	}
}
