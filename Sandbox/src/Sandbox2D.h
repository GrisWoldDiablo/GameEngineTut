#pragma once
#include "Hazel.h"

class Sandbox2D : public Hazel::Layer
{
public:
	Sandbox2D();
	~Sandbox2D() = default;
	void OnAttach() override;
	void OnDetach() override;

	void OnUpdate(Hazel::Timestep timestep) override;

	void OnImGuiRender(Hazel::Timestep timestep) override;
	void OnEvent(Hazel::Event& event) override;

private:
	void CalculateFPS(Hazel::Timestep timestep);
	void UpdateSquareList();
	void DrawMainGui();
	void DrawSquaresGui();

	void CreateSquares();
	void CreateSquare(int amount);

	// Z sorting since squares are transparent
	inline void SortSquares()
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

	Hazel::Ref<Hazel::Texture2D> _checkerboardTexture;

	glm::vec4 _clearColor = { 0.13f, 0.0f, 0.3f, 1.0f };
	struct Square
	{
		glm::vec3 Position;
		glm::vec2 Size;
		glm::vec4 Color;
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
};
