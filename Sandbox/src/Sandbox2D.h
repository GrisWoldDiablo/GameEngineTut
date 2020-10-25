#pragma once
#include "Hazel.h"

#include "ParticleSystem.h"

class Sandbox2D final : public Hazel::Layer
{
public:
	Sandbox2D();
	~Sandbox2D() = default;
	void OnAttach() override;
	void OnDetach() override;

	void OnUpdate(Hazel::Timestep timestep) override;

	void SafetyShutdownCheck();

	void OnImGuiRender(Hazel::Timestep timestep) override;
	void OnEvent(Hazel::Event& event) override;

private:
	void CalculateFPS(Hazel::Timestep timestep);
	void DrawMainGui();
	void DrawStats(Hazel::Timestep timestep);
	
private:
	Hazel::OrthographicCameraController _cameraController;

	Hazel::Color _clearColor = { 0.13f, 0.13f, 0.13f, 1.0f };

	Hazel::Ref<Hazel::Framebuffer> _framebuffer;

	Hazel::Ref<Hazel::Texture2D> _spriteSheet;


	// FPS
	int _frameCount = 0;
	int _currentFPS = 60;
	float _oneSecondCountDown = 1.0f;
	int _lowFrames = 0;
	Hazel::InstrumentationTimer _updateTimer;
};
