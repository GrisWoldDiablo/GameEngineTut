#include "hzpch.h"
#include "Scene.h"
#include "components.h"
#include "Hazel/Renderer/Renderer2D.h"

#include "Entity.h"

namespace Hazel
{
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

	void Scene::DestroyEntity(Entity entity)
	{
		_registry.destroy(entity);
	}

	void Scene::OnUpdate(Timestep timestep)
	{
		// Update Scripts
		_registry.view<NativeScriptComponent>().each([=](auto entity, auto& nsc)
		{
			auto& instance = nsc.Instance;

			// TODO Move to OnScenePlay
			if (instance == nullptr)
			{
				instance = nsc.InstantiateScript();
				instance->_entity = { entity, this };
				instance->OnCreate();
			}

			if (!instance->IsEnable)
			{
				return;
			}

			instance->OnUpdate(timestep);
		});

		// Render 2D
		Camera* mainCamera = nullptr;
		glm::mat4 cameraTransform;
		_registry.view<CameraComponent, TransformComponent>().each([&](auto entity, auto& camera, auto& transform)
		{
			if (camera.IsPrimary)
			{
				mainCamera = &camera.Camera;
				cameraTransform = transform.GetTransform();
				return false;
			}
			return true;
		});

		if (mainCamera == nullptr)
		{
			return;
		}

		Renderer2D::BeginScene(*mainCamera, cameraTransform);

		auto group = _registry.group<TransformComponent>(entt::get<SpriteRendererComponent>);

		group.sort<TransformComponent>([](const auto& lhs, const auto& rhs)
		{
			return lhs.Position.z < rhs.Position.z;
		});

		for (auto entity : group)
		{
			auto [transform, sprite] = group.get<TransformComponent, SpriteRendererComponent>(entity);
			if (sprite.Texture == nullptr)
			{
				Renderer2D::DrawQuad(transform.GetTransform(), sprite.Color);
			}
			else
			{
				Renderer2D::DrawQuad(transform.GetTransform(), sprite.Texture, sprite.Tiling, sprite.Color);
			}
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
			if (!cameraComponent.IsFixedAspectRatio || cameraComponent.Camera.GetProjectionType() == SceneCamera::ProjectionType::Perspective)
			{
				cameraComponent.Camera.SetViewportSize(width, height);
			}
		}
	}

	template<typename T>
	void Scene::OnComponentAdded(Entity entity, T& component)
	{
		static_assert(false);
	}

	template<>
	void Scene::OnComponentAdded<TagComponent>(Entity entity, TagComponent& component)
	{}

	template<>
	void Scene::OnComponentAdded<TransformComponent>(Entity entity, TransformComponent& component)
	{}

	template<>
	void Scene::OnComponentAdded<SpriteRendererComponent>(Entity entity, SpriteRendererComponent& component)
	{}

	template<>
	void Scene::OnComponentAdded<CameraComponent>(Entity entity, CameraComponent& component)
	{
		component.Camera.SetViewportSize(_viewportWidth, _viewportHeight);
	}

	template<>
	void Scene::OnComponentAdded<NativeScriptComponent>(Entity entity, NativeScriptComponent& component)
	{}
}
