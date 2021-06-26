#pragma once
#include "Timestep.h"
#include "Base.h"
#include "Hazel/Events/Event.h"

namespace Hazel
{
	class Layer
	{
	public:
		Layer(std::string debugName = "Layer");
		virtual ~Layer() = default;

		virtual void OnAttach() {}
		virtual void OnDetach() {}
		virtual void OnUpdate() {}
		virtual void OnImGuiRender() {}
		virtual void OnEvent(Event& event) {}

		const std::string& GetName() const { return _debugName; }

	protected:
		std::string _debugName;
	};
}
