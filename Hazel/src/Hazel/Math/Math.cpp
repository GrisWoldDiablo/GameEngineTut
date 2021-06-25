#include "hzpch.h"
#include "Math.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/matrix_decompose.hpp>

namespace Hazel::Math
{
	bool DecomposeTransform(const glm::mat4& transform, glm::vec3& outPosition, glm::vec3& outRotation, glm::vec3& outScale)
	{
		// From glm::decompose in matrix_decompose.inl

		using namespace glm;

		mat4 LocalMatrix(transform);

		// Normalize the matrix.
		if (epsilonEqual(LocalMatrix[3][3], 0.0f, epsilon<float>()))
		{
			return false;
		}

		// First, isolate perspective. This is the messiest
		if (epsilonNotEqual(LocalMatrix[0][3], 0.0f, epsilon<float>())
			|| epsilonNotEqual(LocalMatrix[1][3], 0.0f, epsilon<float>())
			|| epsilonNotEqual(LocalMatrix[2][3], 0.0f, epsilon<float>()))
		{
			// Clear the perspective partition
			LocalMatrix[0][3] = LocalMatrix[1][3] = LocalMatrix[2][3] = 0.0f;
			LocalMatrix[3][3] = 1.0f;
		}

		// Next take care of position (easy).
		outPosition = vec3(LocalMatrix[3]);
		LocalMatrix[3] = vec4(0.0f, 0.0f, 0.0f, LocalMatrix[3].w);

		vec3 Row[3];// , Pdum3;

		// Now get scale and shear. *(shear not used in this method mod)
		for (int i = 0; i < 3; i++)
		{
			for (int j = 0; j < 3; j++)
			{
				Row[i][j] = LocalMatrix[i][j];
			}
		}

		// Compute X scale factor and normalize first row.
		for (int i = 0; i < 3; i++)
		{
			outScale[i] = length(Row[i]);
			Row[i] = detail::scale(Row[i], 1.0f);
		}

		// At this point, the matrix (in rows[]) is orthonormal.
		// Check for coordinate system flip. If the determinant
		// is -1, then negate the matrix and scaling factors.

#if 0
		Pdum3 = cross(Row[1], Row[2]);
		if (dot(Row[0], Pdum3) < 0.0f)
		{
			for (int i = 0; i < 3; i++)
			{
				outScale[i] *= -1.0f;
				Row[i] *= -1.0f;
			}
		}
#endif // 0

		outRotation.y = asin(-Row[0][2]);
		if (cos(outRotation.y) != 0.0f)
		{
			outRotation.x = atan2(Row[1][2], Row[2][2]);
			outRotation.z = atan2(Row[0][1], Row[0][0]);
		}
		else
		{
			outRotation.x = atan2(-Row[2][0], Row[1][1]);
			outRotation.z = 0.0f;
		}

		return true;
	}
}
