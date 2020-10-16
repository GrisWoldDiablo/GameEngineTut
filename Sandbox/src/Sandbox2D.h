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
	void UpdateSquareList();
	void DrawMainGui();
	void DrawSquaresGui();
	void DrawStats(Hazel::Timestep timestep);
	void DrawParticlesGui();
	
	void CreateSquares();
	void CreateSquare(int amount);

	// Z sorting since squares are transparent
	void SortSquares()
	{
		HZ_PROFILE_FUNCTION();
		std::sort(_squares.begin(), _squares.end(),
			[](const Hazel::Ref<Square> left, const Hazel::Ref<Square> right)
			{
				return left->Position.z < right->Position.z;
			});
	}
private:
	Hazel::OrthographicCameraController _cameraController;

	// Temporary
	Hazel::Ref<Hazel::VertexArray> _squareVertexArray;
	Hazel::Ref<Hazel::Shader> _flatColorShader;

	Hazel::Ref<Hazel::Texture2D> _unwrapTexture;
	Hazel::Ref<Hazel::Texture2D> _checkerboardTexture;
	Hazel::Ref<Hazel::Texture2D> _logoTexture;
	Hazel::Ref<Hazel::Texture2D> _spriteSheet;

	Hazel::Color _clearColorA = { 1.0f, 1.0f, 1.0f, 1.0f };
	Hazel::Color _clearColorB = { 0.13f, 0.13f, 0.13f, 1.0f };
	Hazel::Color _lerpedColor;
	float _lerpValueSin = 1.0f;
	float _lerpValueCos = 1.0f;
	float _lerpSpeed = 1.0f;
	struct Square
	{
		glm::vec3 Position;
		glm::vec2 Size;
		Hazel::Color Color;
	};
	std::vector<Hazel::Ref<Square>> _squares = std::vector<Hazel::Ref<Square>>();
	int _amountOfSquares = 0;
	int _amountToAdd = 0;
	bool _addSquare = false;
	bool _clearSquares = false;

	// FPS
	int _frameCount = 0;
	int _currentFPS = 60;
	float _oneSecondCountDown = 1.0f;
	int _lowFrames = 0;

	// Multi-Threading 
	std::mutex _mutex;
	std::vector<std::thread> _squareCreationThreads = std::vector<std::thread>();
	bool _shouldCreateSquares = false;
	bool _isCreatingSquares = false;

	// Particle
	ParticleSystem _particleSystem;
	ParticleProps _particleProps;
	Hazel::InstrumentationTimer _updateTimer;
	int _particlesAmountPerFrame = 50;
	int _particlesPoolSize = 100000;
};
