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
			HZ_CORE_LINFO("Create {0}", _entity.Name());
		}

		void OnUpdate() override
		{
			if (!GetComponent<CameraComponent>().IsPrimary
				|| !Input::IsMouseButtonPressed(MouseCode::ButtonRight))
			{
				return;
			}

			auto& position = _entity.Transform().Position;

			float speed = 5.0f;
			float timestep = Time::GetTimestep();

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

			auto& camera = GetComponent<CameraComponent>().Camera;
			if (camera.GetProjectionType() == SceneCamera::ProjectionType::Perspective)
			{
				if (Input::IsKeyPressed(KeyCode::E))
				{
					position.z -= speed * timestep;
				}
				if (Input::IsKeyPressed(KeyCode::Q))
				{
					position.z += speed * timestep;
				}
			}
			else
			{
				auto orthographicSize = camera.GetOrthographicSize();
				if (Input::IsKeyPressed(KeyCode::E))
				{
					orthographicSize -= speed * timestep;
					camera.SetOrthographicSize(orthographicSize);
				}
				if (Input::IsKeyPressed(KeyCode::Q))
				{
					orthographicSize += speed * timestep;
					camera.SetOrthographicSize(orthographicSize);
				}
			}
		}

		std::string GetClassFilePath() override
		{
			return std::string(__FILE__);
		}
	};
}
