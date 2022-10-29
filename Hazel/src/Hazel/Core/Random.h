#pragma once

#include <glm/glm.hpp>
#include <random>

namespace Hazel
{
	class Random
	{
	public:
		/// <summary>
		/// Initialize the Random machine to a specific seed
		/// </summary>
		/// <param name="seed">seed</param>
		Random(int seed = static_cast<int>(std::time(nullptr)));
		~Random() = default;

		/// <summary>
		/// Get random float between 0.0f - 1.0f
		/// </summary>
		static float Float()
		{
			return GetInstance()->FloatImpl();
		}

		/// <summary>
		/// Get random double between 0.0 - 1.0
		/// </summary>
		static double Double()
		{
			return GetInstance()->DoubleImpl();
		}

		/// <summary>
		/// Get a random integer between min and max exclusive.
		/// </summary>
		/// <param name="min">Minimum of the range</param>
		/// <param name="max">Maximum of the range</param>
		static int32_t Range(int32_t min, int32_t max)
		{
			return GetInstance()->RangeImpl(min, max);
		}

		/// <summary>
		/// Get a random float between min and max inclusive.
		/// </summary>
		/// <param name="min">Minimum of the range</param>
		/// <param name="max">Maximum of the range</param>
		static float Range(float min, float max)
		{
			return GetInstance()->RangeImpl(min, max);
		}

		/// <summary>
		/// Get a random double between min and max inclusive.
		/// </summary>
		/// <param name="min">Minimum of the range</param>
		/// <param name="max">Maximum of the range</param>
		static double Range(double min, double max)
		{
			return GetInstance()->RangeImpl(min, max);
		}

		/// <summary>
		/// Get Random normalized vec2
		/// </summary>
		static glm::vec2 Vec2()
		{
			return GetInstance()->Vec2Impl();
		}
		/// <summary>
		/// Get Random normalized vec3
		/// </summary>
		static glm::vec3 Vec3()
		{
			return GetInstance()->Vec3Impl();
		}

		/// <summary>
		/// Get Random normalized vec4
		/// </summary>
		static glm::vec4 Vec4()
		{
			return GetInstance()->Vec4Impl();
		}

		/// <summary>
		/// Get Random vec2 with range
		/// </summary>
		static glm::vec2 RangeVec2(const glm::vec2& x, const glm::vec2& y)
		{
			return GetInstance()->RangeVec2Impl(x, y);
		}

		/// <summary>
		/// Get Random vec3 with range
		/// </summary>
		static glm::vec3 RangeVec3(const glm::vec2& x, const glm::vec2& y, const glm::vec2& z)
		{
			return GetInstance()->RangeVec3Impl(x, y, z);
		}

		/// <summary>
		/// Get Random vec4 with range
		/// </summary>
		static glm::vec4 RangeVec4(const glm::vec2& x, const glm::vec2& y, const glm::vec2& z, const glm::vec2& w)
		{
			return GetInstance()->RangeVec4Impl(x, y, z, w);
		}

	private:
		static Random* GetInstance();
		
		float FloatImpl();
		double DoubleImpl();
		int32_t RangeImpl(int32_t min, int32_t max);
		float RangeImpl(float min, float max);
		double RangeImpl(double min, double max);
		glm::vec2 Vec2Impl();
		glm::vec3 Vec3Impl();
		glm::vec4 Vec4Impl();
		glm::vec2 RangeVec2Impl(const glm::vec2& x, const glm::vec2& y);
		glm::vec3 RangeVec3Impl(const glm::vec2& x, const glm::vec2& y, const glm::vec2& z);
		glm::vec4 RangeVec4Impl(const glm::vec2& x, const glm::vec2& y, const glm::vec2& z, const glm::vec2& w);

	private:
		std::mt19937 _mersenneTwister;
		std::uniform_real_distribution<double> _uniformDouble = std::uniform_real_distribution(0.0, 1.0);

		// Singleton related 
		static Random* _sInstance;
	};
}
