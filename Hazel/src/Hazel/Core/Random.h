#pragma once

#include "Core.h"

#include <glm/glm.hpp>
#include <random>

namespace Hazel
{
	class Random
	{
	public:
		Random(int seed = std::time(nullptr));
		~Random() = default;
		
		/// <summary>
		/// Get random float between 0.0f - 1.0f
		/// </summary>
		inline static float Float() { Init(); return _sInstance->FloatImpl(); }

		/// <summary>
		/// Get random double between 0.0 - 1.0
		/// </summary>
		inline static double Double() { Init(); return _sInstance->DoubleImpl(); }
		
		/// <summary>
		/// Get a random integer between min and max exclusive.
		/// </summary>
		/// <param name="min">Minimum of the range</param>
		/// <param name="max">Maximum of the range</param>
		inline static int32_t Range(int32_t min, int32_t max) { Init(); return _sInstance->RangeImpl(min, max); }
		
		/// <summary>
		/// Get a random float between min and max inclusive.
		/// </summary>
		/// <param name="min">Minimum of the range</param>
		/// <param name="max">Maximum of the range</param>
		inline static float Range(float min, float max) { Init(); return _sInstance->RangeImpl(min, max); }
		
		/// <summary>
		/// Get a random double between min and max inclusive.
		/// </summary>
		/// <param name="min">Minimum of the range</param>
		/// <param name="max">Maximum of the range</param>
		inline static double Range(double min, double max) { Init(); return _sInstance->RangeImpl(min, max); }
		
		/// <summary>
		/// Get Random normalized vec2
		/// </summary>
		inline static glm::vec2 Vec2() { Init(); return _sInstance->Vec2Impl(); }

		/// <summary>
		/// Get Random normalized vec3
		/// </summary>
		inline static glm::vec3 Vec3() { Init(); return _sInstance->Vec3Impl(); }

		/// <summary>
		/// Get Random normalized vec4
		/// </summary>
		inline static glm::vec4 Vec4() { Init(); return _sInstance->Vec4Impl(); }

	private:
		float FloatImpl();
		double DoubleImpl();
		int32_t RangeImpl(int32_t min, int32_t max);
		float RangeImpl(float min, float max);
		double RangeImpl(double min, double max);
		glm::vec2 Vec2Impl();
		glm::vec3 Vec3Impl();
		glm::vec4 Vec4Impl();

	private:
		std::mt19937 _mersenneTwister;
		std::uniform_int_distribution<int> _uniformIntDistribution;
		std::uniform_real_distribution<float> _uniformFloatDistribution;
		std::uniform_real<double> _uniformDouble = std::uniform_real(0.0);
		std::uniform_real_distribution<double> _uniformDoubleDistribution;

		// Singleton related 
		static void Init();
		static Random* _sInstance;
	};
}
