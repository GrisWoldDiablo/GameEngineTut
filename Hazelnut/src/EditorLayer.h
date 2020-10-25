#pragma once
#include "Hazel.h"

namespace Hazel
{

	class EditorLayer final : public Layer
	{
	public:
		EditorLayer();
		~EditorLayer() = default;
		void OnAttach() override;
		void OnDetach() override;

		void OnUpdate(Timestep timestep) override;

		void SafetyShutdownCheck();

		void OnImGuiRender(Timestep timestep) override;
		void OnEvent(Event& event) override;

	private:
		void CalculateFPS(Timestep timestep);
		void DrawSceneViewport();
		void DrawStats(Timestep timestep);

	private:
		OrthographicCameraController _cameraController;

		Color _clearColor = { 0.13f, 0.13f, 0.13f, 1.0f };

		Ref<Framebuffer> _framebuffer;

		Ref<Texture2D> _spriteSheet;

		// FPS
		int _frameCount = 0;
		int _currentFPS = 60;
		float _oneSecondCountDown = 1.0f;
		int _lowFrames = 0;
		InstrumentationTimer _updateTimer;
	};

}
