#pragma once
#include "Hazel.h"
#include "Level.h"

class GameLayer final : public Hazel::Layer
{
public:
	GameLayer();
	virtual ~GameLayer() = default;

	virtual void OnAttach() override;
	virtual void OnDetach() override;
	 
	virtual void OnUpdate(Hazel::Timestep timestep) override;
	 
	virtual void OnImGuiRender() override;
	virtual void OnEvent(Hazel::Event& event) override;

private:
	Hazel::OrthographicCameraController _cameraController;
	Level _level;
};
