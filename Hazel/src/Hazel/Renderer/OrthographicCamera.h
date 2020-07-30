#pragma once

#include <glm/glm.hpp>

namespace Hazel
{
	class OrthographicCamera
	{
	public:
		OrthographicCamera(float left, float right, float bottom, float top, glm::vec3 position = {0.0f,0.0f,0.0f}, float rotation = 0.0f);
		~OrthographicCamera() = default;

		const glm::vec3& GetPosition() { return _position; }
		inline void SetPosition(const glm::vec3 position)
		{
			_position = position;
			RecalculateViewMatrix();
		}

		float GetRotation() { return _rotation; }
		inline void SetRotation(float rotation)
		{
			_rotation = rotation;
			RecalculateViewMatrix();
		}

		const glm::mat4& GetProjectionMatrix() { return _projectionMatrix; }
		const glm::mat4& GetViewMatrix() { return _viewMatrix; }
		const glm::mat4& GetViewProjectionMatrix() { return _viewProjectionMatrix; }
		
	private:
		void RecalculateViewMatrix();
		
	private:
		glm::mat4 _projectionMatrix;
		glm::mat4 _viewMatrix;
		glm::mat4 _viewProjectionMatrix;

		glm::vec3 _position;
		float _rotation;
	};
}
