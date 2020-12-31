#pragma once
#include "Hazel/Core/Input.h"
#include "Hazel/Core/Log.h"
#include "Hazel/Scene/ScriptableEntity.h"
#include "Hazel/Scene/Components.h"

namespace Hazel
{
	class CameraController : public ScriptableEntity
	{
	public:
		void OnCreate() override
		{
			HZ_CORE_LINFO("Create {0}", GetComponent<TagComponent>().Tag);
		}

		void OnUpdate(Timestep timestep) override
		{
			if (!GetComponent<CameraComponent>().IsPrimary)
			{
				return;
			}

			auto& position = GetComponent<TransformComponent>().Position;
			float speed = 5.0f;

			if (Input::IsKeyPressed(KeyCode::A))
			{
				position.x -= speed * timestep;
			}
			if (Input::IsKeyPressed(KeyCode::D))
			{
				position.x += speed * timestep;
			}

			if (Input::IsKeyPressed(KeyCode::W))
			{
				position.y += speed * timestep;
			}
			if (Input::IsKeyPressed(KeyCode::S))
			{
				position.y -= speed * timestep;
			}

			if (Input::IsKeyPressed(KeyCode::E))
			{
				position.z -= speed * timestep;
			}
			if (Input::IsKeyPressed(KeyCode::Q))
			{
				position.z += speed * timestep;
			}
		}

		std::string GetClassFilePath() override
		{
			return std::string(__FILE__);
		}
	};
}
