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
		HZ_PROFILE_FUNCTION();

		return (float)_uniformDouble(_mersenneTwister);
	}

	double Random::DoubleImpl()
	{
		HZ_PROFILE_FUNCTION();

		return _uniformDouble(_mersenneTwister);
	}

	int32_t Random::RangeImpl(int32_t min, int32_t max)
	{
		HZ_PROFILE_FUNCTION();

		_uniformIntDistribution = std::uniform_int_distribution<int>(min, max - 1);
		return _uniformIntDistribution(_mersenneTwister);
	}

	float Random::RangeImpl(float min, float max)
	{
		HZ_PROFILE_FUNCTION();

		_uniformFloatDistribution = std::uniform_real_distribution<float>(min, max);
		return _uniformFloatDistribution(_mersenneTwister);
	}

	double Random::RangeImpl(double min, double max)
	{
		HZ_PROFILE_FUNCTION();

		_uniformDoubleDistribution = std::uniform_real_distribution<double>(min, max);
		return _uniformFloatDistribution(_mersenneTwister);
	}

	glm::vec2 Random::Vec2Impl()
	{
		HZ_PROFILE_FUNCTION();

		return { FloatImpl(),FloatImpl() };
	}

	glm::vec3 Random::Vec3Impl()
	{
		HZ_PROFILE_FUNCTION();

		return { FloatImpl(), FloatImpl(), FloatImpl() };
	}

	glm::vec4 Random::Vec4Impl()
	{
		HZ_PROFILE_FUNCTION();

		return { FloatImpl(), FloatImpl(), FloatImpl(), FloatImpl() };
	}

	glm::vec2 Random::RangeVec2Impl(const glm::vec2& x, const glm::vec2& y)
	{
		HZ_PROFILE_FUNCTION();

		return { RangeImpl(x.x,x.y), RangeImpl(y.x,y.y) };
	}

	glm::vec3 Random::RangeVec3Impl(const glm::vec2& x, const glm::vec2& y, const glm::vec2& z)
	{
		HZ_PROFILE_FUNCTION();

		return { RangeImpl(x.x,x.y), RangeImpl(y.x,y.y), RangeImpl(z.x,z.y) };
	}
	
	glm::vec4 Random::RangeVec4Impl(const glm::vec2& x, const glm::vec2& y, const glm::vec2& z, const glm::vec2& w)
	{
		HZ_PROFILE_FUNCTION();

		return { RangeImpl(x.x,x.y), RangeImpl(y.x,y.y), RangeImpl(z.x,z.y), RangeImpl(w.x,w.y) };
	}
}
