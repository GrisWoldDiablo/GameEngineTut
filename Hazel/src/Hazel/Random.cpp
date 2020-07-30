#include "hzpch.h"
#include "Random.h"

namespace Hazel
{
	// Static singleton access
	Random* Random::_sInstance = nullptr;

	Random::Random(int seed)
		:_mersenneTwister(seed)
	{
	}

	void Random::Init()
	{
		if (_sInstance == nullptr)
		{
			_sInstance = new Random();
		}
	}

	float Random::GetRandomImpl()
	{
		_uniformRealDistribution = std::uniform_real_distribution<float>(0.0f, 1.0f);
		return _uniformRealDistribution(_mersenneTwister);
	}

	int Random::GetRandomRangeImpl(int min, int max)
	{
		_uniformIntDistribution = std::uniform_int_distribution<int>(min, max - 1);
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
