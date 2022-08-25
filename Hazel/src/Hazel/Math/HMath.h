#pragma once

#include <glm/glm.hpp>

namespace Hazel
{
	class HMath
	{
	public:
		static bool IsNearlyZero(float A);
		static bool IsNearlyEqual(float A, float B);
		static bool IsNearlyNotEqual(float A, float B);
		static bool IsNearlyZero(double A);
		static bool IsNearlyEqual(double A, double B);
		static bool IsNearlyNotEqual(double A, double B);

		template<typename T>
		static bool IsNearlyZero(T A)
		{
			return glm::all(glm::epsilonEqual(A, glm::zero<T>(), glm::epsilon<float>()));
		}

		template<typename T>
		static bool IsNearlyEqual(T A, T B)
		{
			return glm::all(glm::epsilonEqual(A, B, glm::epsilon<float>()));
		}

		template<typename T>
		static bool IsNearlyNotEqual(T A, T B)
		{
			return glm::all(glm::epsilonNotEqual(A, B, glm::epsilon<float>()));
		}

		static bool DecomposeTransform(const glm::mat4& transform, glm::vec3& outPosition, glm::vec3& outRotation, glm::vec3& outScale);
	};
}
