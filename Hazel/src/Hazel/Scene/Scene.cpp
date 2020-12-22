#include "hzpch.h"
#include "Scene.h"
#include "components.h"
#include "Hazel/Renderer/Renderer2D.h"

#include "Entity.h"

namespace Hazel
{
	Scene::Scene()
	{

	}

	Scene::~Scene()
	{
		_registry.clear();
	}

	Entity Scene::CreateEntity(std::string name)
	{
		Entity entity = { _registry.create(), this };

		entity.AddComponent<TransformComponent>();

		auto& tagComponent = entity.AddComponent<TagComponent>();
		tagComponent.Tag = name.empty() ? "Entity" : std::move(name);

		return entity;
	}

	void Scene::OnUpdate(Timestep timestep)
	{
		// Render 2D
		Camera* mainCamera = nullptr;
		glm::mat4* cameraTransform = nullptr;
		{
			_registry.view<CameraComponent, TransformComponent>().each([&](auto entity, auto& camera, auto& transform)
				{
					if (camera.IsPrimary)
					{
						mainCamera = &camera.Camera;
						cameraTransform = &transform.Transform;
						return false;
					}
				});
		}

		if (mainCamera == nullptr)
		{
			return;
		}

		Renderer2D::BeginScene(*mainCamera, *cameraTransform);

		auto group = _registry.group<TransformComponent>(entt::get<SpriteRendererComponent>);
		for (auto entity : group)
		{
			auto& [transform, sprite] = group.get<TransformComponent, SpriteRendererComponent>(entity);
			Renderer2D::DrawQuad(transform, sprite.Color);
		}

		Renderer2D::EndScene();
	}

	void Scene::OnViewportResize(uint32_t width, uint32_t height)
	{
		_viewportWidth = width;
		_viewportHeight = height;
		
		// Resize our non-FixedAspectRation cameras
		auto view = _registry.view<CameraComponent>();
		for (auto entity : view)
		{
			auto& cameraComponent = view.get<CameraComponent>(entity);
			if (!cameraComponent.IsFixedAspectRatio)
			{
				cameraComponent.Camera.SetViewportSize(width, height);
			}
		}
	}

}