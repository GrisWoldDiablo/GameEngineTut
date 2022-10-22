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
		virtual void OnCreate() override
		{
			HZ_CORE_LINFO("Create {0}", _entity.Name());
		}

		virtual void OnUpdate(Timestep timestep) override
		{
			if (!GetComponent<CameraComponent>().IsPrimary
				|| !Input::IsMouseButtonDown(MouseCode::ButtonRight))
			{
				return;
			}

			auto& position = _entity.Transform().Position;

			float speed = 5.0f;

			if (Input::IsKeyDown(KeyCode::A))
			{
				position.x -= speed * timestep;
			}
			if (Input::IsKeyDown(KeyCode::D))
			{
				position.x += speed * timestep;
			}

			if (Input::IsKeyDown(KeyCode::W))
			{
				position.y += speed * timestep;
			}
			if (Input::IsKeyDown(KeyCode::S))
			{
				position.y -= speed * timestep;
			}

			auto& camera = GetComponent<CameraComponent>().Camera;
			if (camera.GetProjectionType() == SceneCamera::ProjectionType::Perspective)
			{
				if (Input::IsKeyDown(KeyCode::E))
				{
					position.z -= speed * timestep;
				}
				if (Input::IsKeyDown(KeyCode::Q))
				{
					position.z += speed * timestep;
				}
			}
			else
			{
				auto orthographicSize = camera.GetOrthographicSize();
				if (Input::IsKeyDown(KeyCode::E))
				{
					orthographicSize -= speed * timestep;
					camera.SetOrthographicSize(orthographicSize);
				}
				if (Input::IsKeyDown(KeyCode::Q))
				{
					orthographicSize += speed * timestep;
					camera.SetOrthographicSize(orthographicSize);
				}
			}
		}

		virtual std::string GetClassFilePath() override
		{
			return { __FILE__ };
		}
	};
}
