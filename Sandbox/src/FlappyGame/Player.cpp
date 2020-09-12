#include "Player.h"
#include <imgui/imgui.h>

using namespace Hazel;

void Player::LoadAssets()
{
	_shipTexture = Texture2D::Create("assets/textures/Ship.png");
}

void Player::OnUpdate(Hazel::Timestep ts)
{
	if (Input::IsKeyPressed(HZ_KEY_SPACE))
	{
		_velocity.y += _enginePower;
		if (_velocity.y < 0.0f)
		{
			_velocity.y += _enginePower * 2.0f;
		}
	}
	else
	{
		_velocity.y -= _gravity;
	}
	_velocity.y = glm::clamp(_velocity.y, -20.0f, 20.0f);
	_position += _velocity * (float)ts;
}

void Player::OnRender()
{
	Renderer2D::DrawQuad(_position, { 1.0f,1.3f }, GetRotation(), _shipTexture);
}

void Player::OnImGuiRender()
{
	ImGui::DragFloat("Engine Power", &_enginePower, 0.1f);
	ImGui::DragFloat("Gravity", &_gravity, 0.1f);
}

void Player::Reset()
{
	_position = { -10.0f,0.0f };
	_velocity = { 5.0f,0.0f };
}
