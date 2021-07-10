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
		Reset();
	}

	void EditorCamera::OnUpdate()
	{
		auto [mouseX, mouseY] = Input::GetMousePosition();
		const glm::vec2& mousePosition{ mouseX, mouseY };
		glm::vec2 delta = (mousePosition - _initialMousePosition) * 0.003f;
		_initialMousePosition = mousePosition;

		_canScrollZoom = true;
		_isAdjusting = false;
		_isDriving = false;

		if (!_isEnable)
		{
			return;
		}

		if (Input::IsKeyPressed(Key::LeftAlt))
		{
			if (Input::IsMouseButtonPressed(Mouse::ButtonMiddle))
			{
				MousePan(delta);
			}
			else if (Input::IsMouseButtonPressed(Mouse::ButtonLeft))
			{
				MouseRotateAround(delta);
			}
			else if (Input::IsMouseButtonPressed(Mouse::ButtonRight))
			{
				MouseZoom(delta.y);
			}
		}
		else if (Input::IsMouseButtonPressed(Mouse::ButtonRight))
		{
			_canScrollZoom = false;
			_isDriving = true;
			MouseRotateInPlace(delta * _rotationSpeed);
			Drive();
		}
		else if (Input::IsMouseButtonPressed(Mouse::ButtonMiddle))
		{
			MousePan(delta);
		}

		UpdateView();
	}

	void EditorCamera::UpdateView()
	{
		//_rotation = glm::vec3(0.0f); // Lock the camera's rotation
		_focalPoint = CalculateFocalPointPosition();

		auto orientation = GetOrientation();
		_viewMatrix = glm::lookAt(_position, _focalPoint, GetUpDirection());
	}

	std::pair<float, float> EditorCamera::PanSpeed() const
	{
		float x = std::min(_viewportWidth / 1000.0f, 2.4f); // max = 2.4f
		float xFactor = 0.0366f * (x * x) - 0.1778f * x + 0.3021f;
		xFactor *= _distance;

		float y = std::min(_viewportHeight / 1000.0f, 2.4f); // max = 2.4f
		float yFactor = 0.0366f * (y * y) - 0.1778f * y + 0.3021f;
		yFactor *= _distance;

		return { xFactor, yFactor };
	}

	float EditorCamera::ZoomSpeed() const
	{
		float distance = _distance * 0.2f;
		distance = std::max(distance, 0.0f);
		float speed = distance * distance;

		speed = std::min(speed, 100.0f); // max speed = 100.0f

		return speed;
	}

	void EditorCamera::Drive()
	{
		auto delta = glm::vec3(0.0f);
		delta.x = (Input::IsKeyPressed(Key::A) ? -1.0f : Input::IsKeyPressed(Key::D) ? 1.0f : 0);
		delta.y = (Input::IsKeyPressed(Key::E) ? 1.0f : Input::IsKeyPressed(Key::Q) ? -1.0f : 0);
		delta.z = (Input::IsKeyPressed(Key::W) ? -1.0f : Input::IsKeyPressed(Key::S) ? 1.0f : 0);

		if (Input::IsKeyPressed(Key::LeftShift))
		{
			delta *= 2.0f;
		}

		_position += GetOrientation() * delta * DrivingSpeed();
	}


	void EditorCamera::OnEvent(Event& event)
	{
		EventDispatcher dispatcher(event);
		dispatcher.Dispatch<MouseScrolledEvent>(HZ_BIND_EVENT_FN(OnMouseScroll));
	}

	bool EditorCamera::OnMouseScroll(MouseScrolledEvent& event)
	{
		float delta = event.GetYOffset() * 0.1f;
		if (!_canScrollZoom)
		{
			if (_isDriving)
			{
				AdjustDrivingSpeed(delta);
			}
			return false;
		}

		MouseZoom(delta);
		UpdateView();

		return false;
	}

	void EditorCamera::AdjustDrivingSpeed(float delta)
	{
		if (_drivingSpeed > 0.1f && _drivingSpeed + delta <= 0.1f)
		{
			_drivingSpeed = 0.1f;
			delta *= 0.0f;
		}
		else if (_drivingSpeed <= 0.1f)
		{
			delta *= 0.1f;
		}
		_drivingSpeed = std::clamp(_drivingSpeed + delta, 0.01f, 2.0f);
	}

	void EditorCamera::MousePan(const glm::vec2& delta)
	{
		auto [xSpeed, ySpeed] = PanSpeed();
		_position += -GetRightDirection() * delta.x * xSpeed;
		_position += GetUpDirection() * delta.y * ySpeed;

		_isAdjusting = true;
	}

	void EditorCamera::MouseRotateInPlace(const glm::vec2& delta)
	{
		float yawSign = GetUpDirection().y < 0.0f ? 1.0f : -1.0f;
		_rotation.z = 0.0f;
		_rotation.x += -delta.y * _rotationSpeed;
		_rotation.y += yawSign * delta.x * _rotationSpeed;

		_isAdjusting = true;
	}

	void EditorCamera::MouseRotateAround(const glm::vec2& delta)
	{
		MouseRotateInPlace(delta);
		_position = _focalPoint - GetForwardDirection() * _distance;

		_isAdjusting = true;
	}


	void EditorCamera::MouseZoom(float delta)
	{
		float zoomDistance = delta * ZoomSpeed();
		_distance -= zoomDistance;
		if (_distance < 1.0f)
		{
			_distance = 1.0f;
		}
		_position += GetForwardDirection() * zoomDistance;

		_isAdjusting = true;
	}

	void EditorCamera::UpdateProjection()
	{
		_aspectRatio = _viewportWidth / _viewportHeight;
		_projection = glm::perspective(glm::radians(_FOV), _aspectRatio, _nearClip, _farClip);
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

	glm::vec3 EditorCamera::CalculateFocalPointPosition() const
	{
		return _position + GetForwardDirection() * _distance;
	}

	glm::quat EditorCamera::GetOrientation() const
	{
		return glm::quat(_rotation);
	}

	void EditorCamera::Reset()
	{
		_distance = 10.0f;
		_rotation = _focalPoint = glm::vec3(0.0f);
		_position = glm::vec3(0.0f, 0.0f, _distance);
		_drivingSpeed = 1.0f;
		UpdateView();
	}
}
