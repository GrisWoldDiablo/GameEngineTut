#include "hzpch.h"
#include "LayerStack.h"

namespace Hazel
{
	LayerStack::LayerStack() = default;

	LayerStack::~LayerStack()
	{
		HZ_PROFILE_FUNCTION();

		for (auto* layer : _layers)
		{
			layer->OnDetach();
			delete layer;
		}
	}

	void LayerStack::PushLayer(Layer* layer)
	{
		HZ_PROFILE_FUNCTION();

		_layers.emplace(_layers.begin() + _layerInsertIndex, layer);
		_layerInsertIndex++;
	}

	void LayerStack::PushOverlay(Layer* overlay)
	{
		HZ_PROFILE_FUNCTION();

		_layers.emplace_back(overlay);
	}

	void LayerStack::PopLayer(Layer* layer)
	{
		HZ_PROFILE_FUNCTION();
		
		auto it = std::find(_layers.begin(), _layers.end(), layer);
		if (it != _layers.end())
		{
			_layers.erase(it);
			_layerInsertIndex--;
		}
	}

	void LayerStack::PopOverlay(Layer* overlay)
	{
		HZ_PROFILE_FUNCTION();

		auto it = std::find(_layers.begin(), _layers.end(), overlay);
		if (it != _layers.end())
		{
			_layers.erase(it);
		}
	}
}