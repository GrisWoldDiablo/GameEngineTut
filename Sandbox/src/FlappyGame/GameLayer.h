#pragma once
#include "Hazel.h"
#include "Level.h"

class GameLayer final : public Hazel::Layer
{
public:
	GameLayer();
	virtual ~GameLayer() = default;

	void OnAttach() override;
	void OnDetach() override;
	
	void OnUpdate(Hazel::Timestep timestep) override;

	void OnImGuiRender(Hazel::Timestep timestep) override;
	void OnEvent(Hazel::Event& event) override;

private:
	Hazel::OrthographicCameraController _cameraController;
	Level _level;
};
