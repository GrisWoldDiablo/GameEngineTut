#include "hzpch.h"
#include "HMath.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/matrix_decompose.hpp>

namespace Hazel
{
	bool HMath::IsNearlyZero(float A)
	{
		return glm::epsilonEqual(A, glm::zero<float>(), glm::epsilon<float>());
	}

	bool HMath::IsNearlyEqual(float A, float B)
	{
		return glm::epsilonEqual(A, B, glm::epsilon<float>());
	}

	bool HMath::IsNearlyNotEqual(float A, float B)
	{
		return glm::epsilonNotEqual(A, B, glm::epsilon<float>());
	}

	bool HMath::IsNearlyZero(double A)
	{
		return glm::epsilonEqual(A, glm::zero<double>(), glm::epsilon<double>());
	}

	bool HMath::IsNearlyEqual(double A, double B)
	{
		return glm::epsilonEqual(A, B, glm::epsilon<double>());
	}

	bool HMath::IsNearlyNotEqual(double A, double B)
	{
		return glm::epsilonNotEqual(A, B, glm::epsilon<double>());
	}

	bool HMath::DecomposeTransform(const glm::mat4& transform, glm::vec3& outPosition, glm::vec3& outRotation, glm::vec3& outScale)
	{
		// From glm::decompose in matrix_decompose.inl
		using namespace glm;

		mat4 LocalMatrix(transform);

		// Normalize the matrix.
		if (IsNearlyZero(LocalMatrix[3][3]))
		{
			return false;
		}

		for (length_t i = 0; i < 4; ++i)
		{
			for (length_t j = 0; j < 4; ++j)
			{
				LocalMatrix[i][j] /= LocalMatrix[3][3];
			}
		}

		// perspectiveMatrix is used to solve for perspective, but it also provides
		// an easy way to test for singularity of the upper 3x3 component.
		mat4 PerspectiveMatrix(LocalMatrix);

		for (length_t i = 0; i < 3; i++)
		{
			PerspectiveMatrix[i][3] = 0.0f;
		}

		PerspectiveMatrix[3][3] = 1.0f;

		/// TODO: Fixme!
		if (IsNearlyZero(determinant(PerspectiveMatrix)))
		{
			return false;
		}

		//vec4 Perspective;
		// First, isolate perspective.  This is the messiest.
		if (IsNearlyNotEqual(LocalMatrix[0][3], 0.0f) ||
			IsNearlyNotEqual(LocalMatrix[1][3], 0.0f) ||
			IsNearlyNotEqual(LocalMatrix[2][3], 0.0f))
		{
			// Clear the perspective partition
			LocalMatrix[0][3] = LocalMatrix[1][3] = LocalMatrix[2][3] = 0.0f;
			LocalMatrix[3][3] = 1.0f;
		}

		// Next take care of translation (easy).
		outPosition = vec3(LocalMatrix[3]);
		LocalMatrix[3] = vec4(0.0f, 0.0f, 0.0f, LocalMatrix[3].w);

		vec3 row[3], Pdum3;

		// Now get scale and shear.
		for (length_t i = 0; i < 3; ++i)
		{
			for (length_t j = 0; j < 3; ++j)
			{
				row[i][j] = LocalMatrix[i][j];
			}
		}


		// Compute X scale factor and normalize first row.
		outScale.x = length(row[0]);// v3Length(Row[0]);

		row[0] = detail::scale(row[0], 1.0f);
		vec3 skew;
		// Compute XY shear factor and make 2nd row orthogonal to 1st.
		skew.z = dot(row[0], row[1]);
		row[1] = detail::combine(row[1], row[0], 1.0f, -skew.z);

		// Now, compute Y scale and normalize 2nd row.
		outScale.y = length(row[1]);
		row[1] = detail::scale(row[1], 1.0f);
		skew.z /= outScale.y;

		// Compute XZ and YZ shears, orthogonalize 3rd row.
		skew.y = dot(row[0], row[2]);
		row[2] = detail::combine(row[2], row[0], 1.0f, -skew.y);
		skew.x = dot(row[1], row[2]);
		row[2] = detail::combine(row[2], row[1], 1.0f, -skew.x);

		// Next, get Z scale and normalize 3rd row.
		outScale.z = length(row[2]);
		row[2] = detail::scale(row[2], 1.0f);
		skew.y /= outScale.z;
		skew.x /= outScale.z;

		// At this point, the matrix (in rows[]) is orthonormal.
		// Check for a coordinate system flip.  If the determinant
		// is -1, then negate the matrix and the scaling factors.
		Pdum3 = cross(row[1], row[2]); // v3Cross(row[1], row[2], Pdum3);
		if (dot(row[0], Pdum3) < 0)
		{
			for (length_t i = 0; i < 3; i++)
			{
				outScale[i] *= -1.0f;
				row[i] *= -1.0f;
			}
		}

		// Now, get the rotations out, as described in the gem.

		quat orientation;

		int i, j, k = 0;
		float root, trace = row[0].x + row[1].y + row[2].z;

		if (trace > 0.0f)
		{
			root = sqrt(trace + 1.0f);
			orientation.w = 0.5f * root;
			root = 0.5f / root;
			orientation.x = root * (row[1].z - row[2].y);
			orientation.y = root * (row[2].x - row[0].z);
			orientation.z = root * (row[0].y - row[1].x);
		}
		else
		{
			static int next[3] = {1, 2, 0};
			i = 0;
			if (row[1].y > row[0].x)
			{
				i = 1;
			}
			
			if (row[2].z > row[i][i])
			{
				i = 2;
			}
			
			j = next[i];
			k = next[j];

			root = sqrt(row[i][i] - row[j][j] - row[k][k] + 1.0f);

			orientation[i] = 0.5f * root;
			root = 0.5f / root;
			orientation[j] = root * (row[i][j] + row[j][i]);
			orientation[k] = root * (row[i][k] + row[k][i]);
			orientation.w = root * (row[j][k] - row[k][j]);
		}

		outRotation = eulerAngles(orientation);
		if (IsNearlyZero(outRotation.y))
		{
			outRotation.y = 0.0f;
		}
		
		return true;
	}
}
