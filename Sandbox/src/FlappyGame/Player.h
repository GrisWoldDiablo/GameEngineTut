#pragma once
#include <Hazel.h>

class Player
{
public:
	Player() = default;

	void LoadAssets();

	void OnUpdate(Hazel::Timestep ts);
	void OnRender();

	void OnImGuiRender();

	void Reset();

	float GetRotation() { return _velocity.y * 4.0f - 90.0f; }
	glm::vec2& GetPosition() { return _position; }

	uint32_t GetScore() const { return (uint32_t)((_position.x + 10.0f) / 10.0f); }

private:
	glm::vec2 _position = { -10.0f,0.0f };
	glm::vec2 _velocity = { 5.0f,0.0f };

	float _enginePower = 1.0f;
	float _gravity = 0.8f;

	Hazel::Ref<Hazel::Texture2D> _shipTexture;
};

