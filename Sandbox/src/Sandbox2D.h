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
	void DrawParticlesGui();
	
private:
	Hazel::OrthographicCameraController _cameraController;

	Hazel::Color _clearColor = { 0.13f, 0.13f, 0.13f, 1.0f };
	
	Hazel::Ref<Hazel::Texture2D> _spriteSheet;
	Hazel::Ref<Hazel::SubTexture2D> _stairsTexture;
	Hazel::Ref<Hazel::SubTexture2D> _barrelTexture;
	Hazel::Ref<Hazel::SubTexture2D> _treeTexture;

	// FPS
	int _frameCount = 0;
	int _currentFPS = 60;
	float _oneSecondCountDown = 1.0f;
	int _lowFrames = 0;

	// Particle
	ParticleSystem _particleSystem;
	ParticleProps _particleProps;
	Hazel::InstrumentationTimer _updateTimer;
	int _particlesAmountPerFrame = 50;
	int _particlesPoolSize = 100000;
};
