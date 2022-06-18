#pragma once

#include <glm/glm.hpp>
#include <random>

namespace Hazel
{
	class Random
	{
	public:
		Random(int seed);
		~Random() = default;

		/// <summary>
		/// Initialize the Random machine to a specific seed
		/// </summary>
		/// <param name="seed">seed</param>
		static void Init(int seed = static_cast<int>(std::time(nullptr)));

		/// <summary>
		/// Get random float between 0.0f - 1.0f
		/// </summary>
		static float Float() { Init(); return _sInstance->FloatImpl(); }

		/// <summary>
		/// Get random double between 0.0 - 1.0
		/// </summary>
		static double Double() { Init(); return _sInstance->DoubleImpl(); }

		/// <summary>
		/// Get a random integer between min and max exclusive.
		/// </summary>
		/// <param name="min">Minimum of the range</param>
		/// <param name="max">Maximum of the range</param>
		static int32_t Range(int32_t min, int32_t max) { Init(); return _sInstance->RangeImpl(min, max); }

		/// <summary>
		/// Get a random float between min and max inclusive.
		/// </summary>
		/// <param name="min">Minimum of the range</param>
		/// <param name="max">Maximum of the range</param>
		static float Range(float min, float max) { Init(); return _sInstance->RangeImpl(min, max); }

		/// <summary>
		/// Get a random double between min and max inclusive.
		/// </summary>
		/// <param name="min">Minimum of the range</param>
		/// <param name="max">Maximum of the range</param>
		static double Range(double min, double max) { Init(); return _sInstance->RangeImpl(min, max); }

		/// <summary>
		/// Get Random normalized vec2
		/// </summary>
		static glm::vec2 Vec2() { Init(); return _sInstance->Vec2Impl(); }
		/// <summary>
		/// Get Random normalized vec3
		/// </summary>
		static glm::vec3 Vec3() { Init(); return _sInstance->Vec3Impl(); }

		/// <summary>
		/// Get Random normalized vec4
		/// </summary>
		static glm::vec4 Vec4() { Init(); return _sInstance->Vec4Impl(); }

		/// <summary>
		/// Get Random vec2 with range
		/// </summary>
		static glm::vec2 RangeVec2(const glm::vec2& x, const glm::vec2& y) { Init(); return _sInstance->RangeVec2Impl(x, y); }

		/// <summary>
		/// Get Random vec3 with range
		/// </summary>
		static glm::vec3 RangeVec3(const glm::vec2& x, const glm::vec2& y, const glm::vec2& z) { Init(); return _sInstance->RangeVec3Impl(x, y, z); }

		/// <summary>
		/// Get Random vec4 with range
		/// </summary>
		static glm::vec4 RangeVec4(const glm::vec2& x, const glm::vec2& y, const glm::vec2& z, const glm::vec2& w) { Init(); return _sInstance->RangeVec4Impl(x, y, z, w); }

	private:
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
		std::uniform_real_distribution<double>_uniformDouble = std::uniform_real_distribution(0.0, 1.0);

		// Singleton related 
		static Random* _sInstance;
	};
}
