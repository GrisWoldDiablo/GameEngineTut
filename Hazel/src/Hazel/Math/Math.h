#pragma once

#include <glm/glm.hpp>

namespace Hazel::Math
{
	template<typename T>
	bool IsNearlyZero(T A)
	{
		return glm::epsilonEqual(A, glm::zero<T>(), glm::epsilon<T>());
	}

	template<typename T>
	bool IsNearlyEqual(T A, T B)
	{
		return glm::epsilonEqual(A, B, glm::epsilon<T>());
	}

	template<typename T>
	bool IsNearlyNotEqual(T A, T B)
	{
		return glm::epsilonNotEqual(A, B, glm::epsilon<T>());
	}

	bool DecomposeTransform(const glm::mat4& transform, glm::vec3& outPosition, glm::vec3& outRotation, glm::vec3& outScale);
}
