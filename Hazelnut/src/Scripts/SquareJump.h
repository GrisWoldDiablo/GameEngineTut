#pragma once
#include "Hazel/Scene/ScriptableEntity.h"

namespace Hazel
{
	class SquareJump : public ScriptableEntity
	{
	public:
		float gravity = 0.1f;
		float speed = 20.0f;
		bool isPressed = false;
		float upward = 0.0f;

		virtual void OnUpdate(Timestep timestep) override
		{
			auto& position = _entity.Transform().Position;
			position.y -= gravity;
			position.y += upward * timestep;
			upward -= speed * timestep;

			if (upward < 0.0f)
			{
				upward = 0.0f;
			}

			if (position.y < 0.0f)
			{
				position.y = 0.0f;
				isPressed = false;
			}

			if (!isPressed && Input::IsKeyPressed(KeyCode::SPACE))
			{
				upward += speed;
				isPressed = true;
			}
		}

		virtual std::string GetClassFilePath() override
		{
			return { __FILE__ };
		}
	};
}
