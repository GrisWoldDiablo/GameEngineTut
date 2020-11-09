#pragma once

#include "Hazel/Core/Layer.h"

namespace Hazel
{
	class ImGuiLayer final : public Layer
	{
	public:
		ImGuiLayer();
		~ImGuiLayer() = default;

		void OnAttach() override;
		void OnDetach() override;
		void OnEvent(Event& event) override;

		void Begin(); // Start a new frame.
		void End(); // Send the new frame

		void BlockEvents(bool block) { _isBlockingEvents = block; }

	private:
		bool _isBlockingEvents = true;
	};
}
