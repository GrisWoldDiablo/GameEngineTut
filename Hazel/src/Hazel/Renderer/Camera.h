#pragma once
#include <glm/glm.hpp>

namespace Hazel
{
	class Camera
	{
	public:
		Camera() = default;
		Camera(const glm::mat4& projection);

		virtual ~Camera() = default;
		const glm::mat4& GetProjection() const { return _projection; }

		virtual const glm::mat4& GetViewMatrix() const = 0;
		virtual glm::mat4 GetViewProjection() const = 0;

		virtual glm::vec3 GetUpDirection() const = 0;
		virtual glm::vec3 GetRightDirection() const = 0;
		virtual glm::vec3 GetForwardDirection() const = 0;
		virtual const glm::vec3& GetPosition() const = 0;
		virtual const glm::vec3& GetRotation() const = 0;
		virtual glm::vec2 GetResolution() const = 0;
		virtual glm::quat GetOrientation() const = 0;

	protected:
		glm::mat4 _projection = glm::mat4(1.0f);
	};
}
