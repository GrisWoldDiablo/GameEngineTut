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
		EditorCamera(float fov, float aspectRatio, float nearClip, float farClip);

		void OnUpdate(Timestep timestep);
		void OnEvent(Event& event);

		inline float GetDistance() const { return _distance; }
		inline void SetDistance(float distance) { _distance = distance; }

		inline void SetViewportSize(float width, float height) { _viewportWidth = width; _viewportHeight = height; UpdateProjection(); }

		const glm::mat4& GetViewMatrix() const { return _viewMatrix; }
		glm::mat4 GetViewProjection() const { return _projection * _viewMatrix; }

		glm::vec3 GetUpDirection() const;
		glm::vec3 GetRightDirection() const;
		glm::vec3 GetForwardDirection() const;
		const glm::vec3& GetPosition() const { return _position; }
		const glm::vec3& GetRotation() const { return _rotation; }
		glm::vec2 GetResolution() const { return { _viewportWidth, _viewportHeight }; }
		glm::quat GetOrientation() const;

		float GetDrivingSpeed() const { return _drivingSpeed; }

		bool IsAdjusting() const { return _isAdjusting; }
		bool IsPanning() const { return _isPanning; }
		bool IsZooming() const { return _isZooming; }
		bool IsDriving()const { return _isDriving; }
		bool& IsEnable() { return _isEnable; }

		float GetNearClip() const { return _nearClip; }
		float GetFarClip() const { return _farClip; }
		void Reset();

	private:
		void UpdateProjection();
		void UpdateView();

		bool OnMouseScroll(const MouseScrolledEvent& mouseScrolledEvent);

		void MousePan(const glm::vec2& delta);
		void MouseRotateAround(const glm::vec2& delta);
		void MouseRotateInPlace(const glm::vec2& delta);
		void MouseZoom(float delta);
		void Drive();

		glm::vec3 CalculateFocalPointPosition() const;

		std::pair<float, float> PanSpeed() const;
		void AdjustDrivingSpeed(float delta);
		float DrivingSpeed() const { return 0.2f * _drivingSpeed; } // Small adjustment value so speed at 1.0f feels right
		float ZoomSpeed() const;

		bool _isEnable = false;
		bool _isPanning = false;
		bool _isZooming = false;

	private:
		float _FOV = 30.0f;
		float _aspectRatio = 1.778f;
		float _nearClip = 0.1f;
		float _farClip = 1000.0f;

		glm::mat4 _viewMatrix = {};
		glm::vec3 _position = { 0.0f, 0.0f, 10.0f };
		glm::vec3 _focalPoint = { 0.0f, 0.0f, 0.0f };

		glm::vec2 _initialMousePosition = {};

		float _distance = 10.0f;
		glm::vec3 _rotation = { 0.0f, 0.0f, 0.0f };

		float _viewportWidth = 1280.0f;
		float _viewportHeight = 720.0f;

		bool _isAdjusting = false;
		bool _isDriving = false;
		bool _canScrollZoom = false;
		float _rotationSpeed = 0.8f;
		float _drivingSpeed = 1.0f;
	};
}
