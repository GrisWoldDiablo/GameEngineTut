#pragma once

#include "Core.h"

#include <glm/glm.hpp>
#include <random>

namespace Hazel
{
	class HAZEL_API Random
	{
	public:
		Random(int seed = std::time(nullptr));

		/// <summary>
		/// Get random float between 0.0f - 1.0f
		/// </summary>
		inline static float GetRandom() { Init(); return _sInstance->GetRandomImpl(); }
		/// <summary>
		/// Get a random integer between min and max exclusive.
		/// </summary>
		/// <param name="min">Minimum of the range</param>
		/// <param name="max">Maximum of the range</param>
		inline static int GetRandomRange(int min, int max) { Init(); return _sInstance->GetRandomRangeImpl(min, max); }
		/// <summary>
		/// Get a random float between min and max inclusive.
		/// </summary>
		/// <param name="min">Minimum of the range</param>
		/// <param name="max">Maximum of the range</param>
		inline static float GetRandomRange(float min, float max) { Init(); return _sInstance->GetRandomRangeImpl(min, max); }
		/// <summary>
		/// Get Random normalized vec2
		/// </summary>
		inline static glm::vec2 GetRandomVec2() { Init(); return _sInstance->GetRandomVec2Impl(); }

		/// <summary>
		/// Get Random normalized vec3
		/// </summary>
		inline static glm::vec3 GetRandomVec3() { Init(); return _sInstance->GetRandomVec3Impl(); }

	private:
		static void Init();
		float GetRandomImpl();
		int GetRandomRangeImpl(int min, int max);
		float GetRandomRangeImpl(float min, float max);
		glm::vec2 GetRandomVec2Impl();
		glm::vec3 GetRandomVec3Impl();

	private:
		std::mt19937 _mersenneTwister;
		std::uniform_int_distribution<int> _uniformIntDistribution;
		std::uniform_real_distribution<float> _uniformRealDistribution;

		static Random* _sInstance;
	};
}
