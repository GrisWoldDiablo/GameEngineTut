#pragma once

#include <glm/glm.hpp>

namespace Hazel::Math
{
	template<typename T>
	bool IsNearlyZero(T A)
	{
		using namespace glm;
		return epsilonEqual(A, glm::zero<T>(), epsilon<T>());
	}

	template<typename T>
	bool IsNearlyEqual(T A, T B)
	{
		using namespace glm;
		return epsilonEqual(A, B, epsilon<T>());
	}

	template<typename T>
	bool IsNearlyNotEqual(T A, T B)
	{
		using namespace glm;

		return epsilonNotEqual(A, B, epsilon<T>());
	}

	bool DecomposeTransform(const glm::mat4& transform, glm::vec3& outPosition, glm::vec3& outRotation, glm::vec3& outScale);
}
