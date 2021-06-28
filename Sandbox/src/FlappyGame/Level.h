#pragma once

#include "Player.h"

struct Pillar
{
	glm::vec3 TopPosition = { 0.0f, 10.0f, 0.0f };
	glm::vec2 TopScale = { 15.0f, 15.0f };

	glm::vec3 BottomPosition = { 10.0f, 10.0f, 0.0f };
	glm::vec2 BottomScale = { 15.0f, 15.0f };
};

class Level
{
public:
	Level() = default;

	void Init();
	void OnUpdate(Hazel::Timestep ts);
	void OnRender();

	void OnImGuiRender();

	Player& GetPlayer() { return _player; }

private:
	void CreatePillar(int index, float offset);
	bool CollisionTest();
	void GameOver();

private:
	Player _player;

	float _pillarTarget = 30.0f;
	int _pillarIndex = 0;
	glm::vec4 _pillarHSV = { 0.0f, 0.8f, 0.8f, 1.0f };

	std::vector<Pillar> _pillars;
	std::vector<glm::vec2> _points;
	Hazel::Ref<Hazel::Texture2D> _triangleTexture;
};
