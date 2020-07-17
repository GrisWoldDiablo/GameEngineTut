#pragma once
#include "Hazel/Core.h"
#include "Hazel/Events/Event.h"

namespace Hazel
{
	class HAZEL_API Layer
	{
	public:
		Layer(const std::string& debugName = "Layer");
		virtual ~Layer();

		virtual void OnAttach() {}
		virtual void OnDetach() {}
		virtual void OnUpdate() {}
		virtual void OnEvent(Event& event) {}

		inline const std::string& GetName() const { return _debugName; }

	protected:
		std::string _debugName;
	};

}
