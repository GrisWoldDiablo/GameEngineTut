#include "hzpch.h"
#include "Layer.h"

namespace Hazel
{
	Layer::Layer(const std::string& debugName)
		: _debugName(debugName) {}

	Layer::~Layer() = default;
}
