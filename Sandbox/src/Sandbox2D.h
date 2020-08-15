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

	void CalculateFPS(Hazel::Timestep timestep);

private:
	Hazel::OrthographicCameraController _cameraController;

	// Temporary
	Hazel::Ref<Hazel::VertexArray> _squareVertexArray;
	Hazel::Ref<Hazel::Shader> _flatColorShader;

	glm::vec4 _clearColor = { 0.13f, 0.0f, 0.3f, 1.0f };
	glm::vec4 _squareColor = { 0.1f, 0.2f, 0.7f, 1.0f };

	// FPS
	int _frameCount = 0;
	int _currentFPS = 0;
	float _oneSecondCountDown = 1.0f;
};
