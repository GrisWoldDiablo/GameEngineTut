#include "hzpch.h"
#include "EditorCamera.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

namespace Hazel
{
	EditorCamera::EditorCamera(float fov, float aspectRatio, float nearClip, float farClip)
		: _FOV(fov), _aspectRatio(aspectRatio), _nearClip(nearClip), _farClip(farClip),
		Camera(glm::perspective(glm::radians(fov), aspectRatio, nearClip, farClip))
	{
		UpdateView();
	}

	void EditorCamera::UpdateProjection()
	{
		_aspectRatio = _viewportWidth / _viewportHeight;
		_projection = glm::perspective(glm::radians(_FOV), _aspectRatio, _nearClip, _farClip);
	}

	void EditorCamera::UpdateView()
	{
		//_yaw = _pitch = 0.0f; // Lock the camera's rotation
		_position = CalculatePosition();

		auto orientation = GetOrientation();
		_viewMatrix = glm::translate(glm::mat4(1.0f), _position) * glm::toMat4(orientation);
		_viewMatrix = glm::inverse(_viewMatrix);
	}

	std::pair<float, float> EditorCamera::PanSpeed() const
	{
		float x = std::min(_viewportWidth / 1000.0f, 2.4f); // max = 2.4f
		float xFactor = 0.0366f * (x * x) - 0.1778f * x + 0.3021f;

		float y = std::min(_viewportHeight / 1000.0f, 2.4f); // max = 2.4f
		float yFactor = 0.0366f * (y * y) - 0.1778f * y + 0.3021f;

		return { xFactor, yFactor };
	}

	float EditorCamera::RotationSpeed() const
	{
		return 0.8f;
	}

	float EditorCamera::ZoomSpeed() const
	{
		float distance = _distance * 0.2f;
		distance = std::max(distance, 0.0f);
		float speed = distance * distance;

		speed = std::min(speed, 100.0f); // max speed = 100.0f

		return speed;
	}

	void EditorCamera::OnUpdate()
	{
		glm::vec2 delta = UpdateMouseDelta();

		_isAdjusting = false;
		if (Input::IsKeyPressed(Key::LeftAlt))
		{
			if (Input::IsMouseButtonPressed(Mouse::ButtonMiddle))
			{
				MousePan(delta);
			}
			else if (Input::IsMouseButtonPressed(Mouse::ButtonLeft))
			{
				MouseRotateAroundFocalPoint(delta);
			}
			else if (Input::IsMouseButtonPressed(Mouse::ButtonRight))
			{
				MouseZoom(delta.y);
			}
		}
		else if (_canMousePan)
		{
			if (Input::IsMouseButtonPressed(Mouse::ButtonLeft))
			{
				MousePan(delta);
			}
		}

		UpdateView();
	}

	glm::vec2 EditorCamera::UpdateMouseDelta()
	{
		auto [mouseX, mouseY] = Input::GetMousePosition();
		const glm::vec2& mousePosition{ mouseX, mouseY };
		glm::vec2 delta = (mousePosition - _initialMousePosition) * 0.003f;
		_initialMousePosition = mousePosition;

		return delta;
	}

	void EditorCamera::OnEvent(Event& event)
	{
		EventDispatcher dispatcher(event);
		dispatcher.Dispatch<MouseScrolledEvent>(HZ_BIND_EVENT_FN(OnMouseScroll));
	}

	bool EditorCamera::OnMouseScroll(MouseScrolledEvent& event)
	{
		float delta = event.GetYOffset() * 0.1f;
		MouseZoom(delta);
		UpdateView();

		return false;
	}

	void EditorCamera::MousePan(const glm::vec2& delta)
	{
		auto [xSpeed, ySpeed] = PanSpeed();
		_focalPoint += -GetRightDirection() * delta.x * xSpeed * _distance;
		_focalPoint += GetUpDirection() * delta.y * ySpeed * _distance;

		_isAdjusting = true;
	}

	void EditorCamera::MouseRotateAroundFocalPoint(const glm::vec2& delta)
	{
		float yawSign = GetUpDirection().y < 0.0f ? 1.0f : -1.0f;
		_yaw += yawSign * delta.x * RotationSpeed();
		_pitch += delta.y * RotationSpeed();

		_isAdjusting = true;
	}

	void EditorCamera::MouseRotateLookAt(const glm::vec2& delta)
	{
		float yawSign = GetUpDirection().y < 0.0f ? 1.0f : -1.0f;
		_yaw += yawSign * delta.x * RotationSpeed();
		_pitch += delta.y * RotationSpeed();

		_isAdjusting = true;
	}

	void EditorCamera::MouseZoom(float delta)
	{
		_distance -= delta * ZoomSpeed();
		if (_distance < 1.0f)
		{
			_focalPoint += GetForwardDirection();
			_distance = 1.0f;
		}

		_isAdjusting = true;
	}

	glm::vec3 EditorCamera::GetUpDirection() const
	{
		return glm::rotate(GetOrientation(), glm::vec3(0.0f, 1.0f, 0.0f));
	}

	glm::vec3 EditorCamera::GetRightDirection() const
	{
		return glm::rotate(GetOrientation(), glm::vec3(1.0f, 0.0f, 0.0f));

	}

	glm::vec3 EditorCamera::GetForwardDirection() const
	{
		return glm::rotate(GetOrientation(), glm::vec3(0.0f, 0.0f, -1.0f));

	}

	glm::vec3 EditorCamera::CalculatePosition() const
	{
		return _focalPoint - GetForwardDirection() * _distance;
	}

	glm::quat EditorCamera::GetOrientation() const
	{
		return glm::quat(glm::vec3(-_pitch, _yaw, 0.0f));
	}
}
