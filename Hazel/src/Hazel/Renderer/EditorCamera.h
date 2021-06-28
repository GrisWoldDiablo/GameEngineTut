#pragma once
#include "Camera.h"
#include "Hazel/Core/Timestep.h"
#include "Hazel/Events/Event.h"
#include "Hazel/Events/MouseEvent.h"

#include <glm/glm.hpp>

namespace Hazel
{
	class EditorCamera : public Camera
	{
	public:
		EditorCamera() = default;
		EditorCamera(float fov, float aspectRation, float nearClip, float farClip);

		void OnUpdate();
		void OnEvent(Event& event);

		inline float GetDistance() const { return _distance; }
		inline void SetDistance(float distance) { _distance = distance; }

		inline void SetViewpostSize(float width, float height) { _viewportWidth = width; _viewportHeight = height; UpdateProjection(); }

		const glm::mat4& GetViewMatrix() const { return _viewMatrix; }
		glm::mat4 GetViewProjection() const { return _projection * _viewMatrix; }

		glm::vec3 GetUpDirection() const;
		glm::vec3 GetRightDirection() const;
		glm::vec3 GetForwardDirection() const;
		const glm::vec3& GetPosition() const { return _position; }
		glm::quat GetOrientation() const;

		float GetPitch() const { return _pitch; }
		float GetYaw() const { return _yaw; }

	private:
		void UpdateProjection();
		void UpdateView();

		bool OnMouseScroll(MouseScrolledEvent& event);

		void MousePan(const glm::vec2& delta);
		void MouseRotateAroundFocalPoint(const glm::vec2& delta);
		void MouseRotateLookAt(const glm::vec2& delta);
		void MouseZoom(float delta);

		glm::vec3 CalculatePosition() const;

		std::pair<float, float> PanSpeed() const;
		float RotationSpeed() const;
		float ZoomSpeed() const;

	private:
		float _FOV = 30.0f;
		float _aspectRatio = 1.778f;
		float _nearClip = 0.1f;
		float _farClip = 1000.0f;

		glm::mat4 _viewMatrix;
		glm::vec3 _position = { 0.0f, 0.0f, 0.0f };
		glm::vec3 _focalPoint = { 0.0f, 0.0f, 0.0f };

		glm::vec2 _initialMousePosition;

		float _distance = 10.0f;
		float _pitch = 0.0f;
		float _yaw = 0.0f;

		float _viewportWidth = 1280.0f;
		float _viewportHeight = 720.0f;
	};
}
