#pragma once

#include "Hazel/Layer.h"

namespace Hazel
{
	class HAZEL_API ImGuiLayer final : public Layer
	{
	public:
		ImGuiLayer();
		~ImGuiLayer();

		virtual void OnAttach() override;
		virtual void OnDetach() override;
		virtual void OnImGuiRender() override; // Generate what is on the frame.

		void Begin(); // Start a new frame.
		void End(); // Send the new frame
	};
}
