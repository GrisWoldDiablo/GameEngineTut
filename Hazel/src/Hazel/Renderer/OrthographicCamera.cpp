#include "hzpch.h"
#include "OrthographicCamera.h"

#include <glm/gtc/matrix_transform.hpp>

namespace Hazel
{
	OrthographicCamera::OrthographicCamera(float left, float right, float bottom, float top, glm::vec3 position, float rotation)
		:_projectionMatrix(glm::ortho(left, right, bottom, top, -1.0f, 1.0f)), _position(position), _rotation(rotation)
	{
		HZ_PROFILE_FUNCTION();

		RecalculateViewMatrix();
	}

	void OrthographicCamera::SetProjection(float left, float right, float bottom, float top)
	{
		HZ_PROFILE_FUNCTION();

		_projectionMatrix = glm::ortho(left, right, bottom, top, -1.0f, 1.0f);
		_viewProjectionMatrix = _projectionMatrix * _viewMatrix;
	}

	void OrthographicCamera::RecalculateViewMatrix()
	{
		HZ_PROFILE_FUNCTION();

		auto identityMatrix = glm::identity<glm::mat4>();
		auto transform = glm::translate(identityMatrix, _position) *
			glm::rotate(identityMatrix, glm::radians(_rotation), glm::vec3(0, 0, 1));

		_viewMatrix = glm::inverse(transform);
		_viewProjectionMatrix = _projectionMatrix * _viewMatrix;
	}
}
