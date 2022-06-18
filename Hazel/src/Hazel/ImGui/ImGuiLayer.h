#pragma once

#include "Hazel/Core/Layer.h"

namespace Hazel
{
	class ImGuiLayer final : public Layer
	{
	public:
		ImGuiLayer();
		virtual ~ImGuiLayer() override = default;

		void OnAttach() override;
		void OnDetach() override;
		void OnEvent(Event& event) override;

		void Begin(); // Start a new frame.
		void End(); // Send the new frame

		void BlockEvents(bool block) { _isBlockingEvents = block; }

		void SetFonts(const std::string& normalFontPath, std::initializer_list<std::string> extraFontsPath);
		void SetDarkThemeColors();

	private:
		bool _isBlockingEvents = true;
	};
}
