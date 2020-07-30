#pragma once

#include "Hazel/Layer.h"

namespace Hazel
{
	class HAZEL_API ImGuiLayer final : public Layer
	{
	public:
		ImGuiLayer();
		~ImGuiLayer();

		void OnAttach() override;
		void OnDetach() override;

		void Begin(); // Start a new frame.
		void End(); // Send the new frame
	};
}
