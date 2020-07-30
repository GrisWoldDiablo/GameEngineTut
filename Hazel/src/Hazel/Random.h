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
		inline static float GetRandom() { return _sInstance->GetRandomImpl(); }
		inline static int GetRandomRange(int min, int max) { return _sInstance->GetRandomRangeImpl(min, max); }
		inline static float GetRandomRange(float min, float max) { return _sInstance->GetRandomRangeImpl(min, max); }
		/// <summary>
		/// Get Random normalized vec2
		/// </summary>
		inline static glm::vec2 GetRandomVec2() { return _sInstance->GetRandomVec2Impl(); }
		
		/// <summary>
		/// Get Random normalized vec3
		/// </summary>
		inline static glm::vec3 GetRandomVec3() { return _sInstance->GetRandomVec3Impl(); }
	private:
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
