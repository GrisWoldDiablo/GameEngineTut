#include "hzpch.h"
#include "Layer.h"

#include <utility>

namespace Hazel
{
	Layer::Layer(std::string debugName)
		: _debugName(std::move(debugName)) {}

	Layer::~Layer() = default;
}
