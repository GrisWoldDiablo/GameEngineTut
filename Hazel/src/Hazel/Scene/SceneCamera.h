#pragma once
#include "Hazel/Renderer/Camera.h"

namespace Hazel
{
	class SceneCamera : public Camera
	{
	public:
		enum class ProjectionType
		{
			Perspective = 0,
			Orthographic = 1,
		};

	public:
		SceneCamera();
		virtual ~SceneCamera() = default;

		void SetOrthographic(float size, float nearClip, float farClip);
		void SetPerspective(float verticalFOV, float nearClip, float farClip);
		void SetViewportSize(uint32_t width, uint32_t height);

		float GetPerspectiveVerticalFOV() const { return _perspectiveFOV; }
		void  SetPerspectiveVerticalFOV(float verticalFOV) { _perspectiveFOV = verticalFOV; RecalculateProjection(); }
		float GetPerspectiveNearClip() const { return _perspectiveNear; }
		void  SetPerspectiveNearClip(float nearClip) { _perspectiveNear = nearClip; RecalculateProjection(); }
		float GetPerspectiveFarClip() const { return _perspectiveFar; }
		void  SetPerspectiveFarClip(float farClip) { _perspectiveFar = farClip; RecalculateProjection(); RecalculateProjection(); }

		float GetOrthographicSize() const { return _orthographicSize; }
		void SetOrthographicSize(float size) { _orthographicSize = size; RecalculateProjection(); }
		float GetOrthographicNearClip() const { return _orthographicNear; }
		void SetOrthographicNearClip(float nearClip) { _orthographicNear = nearClip; RecalculateProjection(); }
		float GetOrthographicFarClip() const { return _orthographicFar; }
		void SetOrthographicFarClip(float farClip) { _orthographicFar = farClip; RecalculateProjection(); RecalculateProjection(); }

		ProjectionType GetProjectionType() const { return _projectionType; }
		void SetProjectionType(ProjectionType type) { _projectionType = type; RecalculateProjection(); }

		const glm::mat4& GetViewMatrix() const override;
		glm::mat4 GetViewProjection() const override;
		glm::vec3 GetUpDirection() const override;
		glm::vec3 GetRightDirection() const override;
		glm::vec3 GetForwardDirection() const override;
		const glm::vec3& GetPosition() const override;
		const glm::vec3& GetRotation() const override;
		glm::vec2 GetResolution() const override;
		glm::quat GetOrientation() const override;

	private:
		void RecalculateProjection();

	private:
		ProjectionType _projectionType = ProjectionType::Perspective;

		float _perspectiveFOV = glm::radians(45.0f);
		float _perspectiveNear = 0.01f;
		float _perspectiveFar = 1000.0f;

		float _orthographicSize = 10.0f;
		float _orthographicNear = -1.0f;
		float _orthographicFar = 1.0f;

		float _aspectRatio = 0.0f;
	};
}
