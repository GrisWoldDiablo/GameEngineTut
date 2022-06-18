#include "hzpch.h"
#include "UUID.h"

#include <random>

namespace Hazel
{
	static std::random_device sRandomDevice;
	static std::mt19937_64 sEngine(sRandomDevice());
	static std::uniform_int_distribution<uint64_t> sUniformDistribution;

	UUID::UUID()
		:_uuid(sUniformDistribution(sEngine))
	{}

	UUID::UUID(uint64_t uuid)
		:_uuid(uuid)
	{}
}
