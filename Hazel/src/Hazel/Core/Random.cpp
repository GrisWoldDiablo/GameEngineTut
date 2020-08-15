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

	float Random::FloatImpl()
	{
		return (float)_uniformDouble(_mersenneTwister);
	}

	double Random::DoubleImpl()
	{
		return _uniformDouble(_mersenneTwister);
	}

	int32_t Random::RangeImpl(int32_t min, int32_t max)
	{
		_uniformIntDistribution = std::uniform_int_distribution<int>(min, max - 1);
		return _uniformIntDistribution(_mersenneTwister);
	}

	float Random::RangeImpl(float min, float max)
	{
		_uniformFloatDistribution = std::uniform_real_distribution<float>(min, max);
		return _uniformFloatDistribution(_mersenneTwister);
	}

	double Random::RangeImpl(double min, double max)
	{
		_uniformDoubleDistribution = std::uniform_real_distribution<double>(min, max);
		return _uniformFloatDistribution(_mersenneTwister);
	}

	glm::vec2 Random::Vec2Impl()
	{
		return { FloatImpl(),FloatImpl() };
	}

	glm::vec3 Random::Vec3Impl()
	{
		return { FloatImpl(), FloatImpl(), FloatImpl() };
	}

	glm::vec4 Random::Vec4Impl()
	{
		return { FloatImpl(), FloatImpl(), FloatImpl(), FloatImpl() };
	}
}
