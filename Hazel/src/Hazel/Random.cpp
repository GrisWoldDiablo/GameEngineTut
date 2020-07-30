#include "hzpch.h"
#include "Random.h"

namespace Hazel
{
	Random* Random::_sInstance = new Random();

	Random::Random(int seed)
		:_mersenneTwister(seed)
	{
	}

	float Random::GetRandomImpl()
	{
		_uniformRealDistribution = std::uniform_real_distribution<float>(0.0f, 1.0f);
		return _uniformRealDistribution(_mersenneTwister);
	}

	int Random::GetRandomRangeImpl(int min, int max)
	{
		_uniformIntDistribution = std::uniform_int_distribution<int>(min, max);
		return _uniformIntDistribution(_mersenneTwister);
	}

	float Random::GetRandomRangeImpl(float min, float max)
	{
		_uniformRealDistribution = std::uniform_real_distribution<float>(min, max);
		return _uniformRealDistribution(_mersenneTwister);
	}

	glm::vec2 Random::GetRandomVec2Impl()
	{
		return { GetRandomImpl(),GetRandomImpl() };
	}

	glm::vec3 Random::GetRandomVec3Impl()
	{
		return { GetRandomImpl(), GetRandomImpl(), GetRandomImpl() };
	}
}
