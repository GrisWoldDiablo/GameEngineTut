#pragma once
#include "Hazel.h"
#include "Hazel/Core/Timer.h"
#include "ParticleSystem.h"

class Sandbox2D final : public Hazel::Layer
{
public:
	Sandbox2D();
	~Sandbox2D() = default;
	void OnAttach() override;
	void OnDetach() override;

	void OnUpdate(const Hazel::Timestep& timestep) override;

	void SafetyShutdownCheck();

	void OnImGuiRender() override;
	void OnEvent(Hazel::Event& event) override;

private:
	void CalculateFPS();
	void DrawMainGui();
	void DrawStats();
	void DrawParticlesGui();

private:
	Hazel::OrthographicCameraController _cameraController;

	Hazel::Color _clearColor = { 0.13f, 0.13f, 0.13f, 1.0f };

	Hazel::Ref<Hazel::Texture2D> _spriteSheet;
	Hazel::Ref<Hazel::SubTexture2D> _fenceTexture;
	Hazel::Ref<Hazel::SubTexture2D> _fencePick;
	Hazel::Ref<Hazel::SubTexture2D> _fenceVert;
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
	Hazel::Timer _updateTimer;
	int _particlesAmountPerFrame = 50;
	int _particlesPoolSize = 100000;

	uint32_t _mapWidth;
	uint32_t _mapHeight;

	std::unordered_map<char, Hazel::Ref<Hazel::SubTexture2D>> _textureMap;
};
