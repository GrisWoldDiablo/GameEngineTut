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

			auto& transform = GetComponent<TransformComponent>().Transform;
			float speed = 5.0f;

			if (Input::IsKeyPressed(KeyCode::A))
			{
				transform[3][0] -= speed * timestep;
			}
			if (Input::IsKeyPressed(KeyCode::D))
			{
				transform[3][0] += speed * timestep;
			}

			if (Input::IsKeyPressed(KeyCode::W))
			{
				transform[3][1] += speed * timestep;
			}
			if (Input::IsKeyPressed(KeyCode::S))
			{
				transform[3][1] -= speed * timestep;
			}

			if (Input::IsKeyPressed(KeyCode::E))
			{
				transform[3][2] -= speed * timestep;
			}
			if (Input::IsKeyPressed(KeyCode::Q))
			{
				transform[3][2] += speed * timestep;
			}
		}

		std::string GetClassFilePath() override
		{
			return std::string(__FILE__);
		}
	};
}
