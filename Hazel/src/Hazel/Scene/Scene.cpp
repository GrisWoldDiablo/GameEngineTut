#include "hzpch.h"
#include "Scene.h"
#include "components.h"
#include "Hazel/Renderer/Renderer2D.h"

namespace Hazel
{
	Scene::~Scene()
	{
		_registry.clear();
	}

	Entity Scene::CreateEntity(std::string name, int tag, int layer)
	{
		Entity entity = { _registry.create(), this };

		auto& baseComponent = entity.AddComponent<BaseComponent>();
		baseComponent.Name = name;
		baseComponent.Tag = tag;
		baseComponent.Layer = layer;

		entity.AddComponent<TransformComponent>();

		return entity;
	}

	void Scene::DestroyEntity(Entity entity)
	{
		_registry.destroy(entity);
	}

	void Scene::OnUpdateRuntime()
	{
		// Update Scripts
		_registry.view<NativeScriptComponent>().each([=](const auto entity, auto& nsc)
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

			instance->OnUpdate();
		});

		// Render 2D
		Camera* mainCamera = nullptr;
		glm::mat4 cameraTransform;
		glm::vec3 cameraPosition;
		_registry.view<CameraComponent, TransformComponent>().each([&](const auto entity, auto& camera, auto& transform)
		{
			if (camera.IsPrimary)
			{
				mainCamera = &camera.Camera;
				cameraTransform = transform.GetTransformMatrix();
				cameraPosition = transform.Position;
				return false;
			}
			return true;
		});

		if (mainCamera == nullptr)
		{
			return;
		}

		if (Renderer2D::BeginScene(*mainCamera, cameraTransform))
		{
			DrawSpriteRenderComponent(cameraPosition);

			Renderer2D::EndScene();
		}
	}

	void Scene::OnUpdateEditor(EditorCamera& camera)
	{
		const auto kIdentityMatrix = glm::mat4(1.0f);
		auto transform = glm::translate(kIdentityMatrix, camera.GetPosition() + camera.GetForwardDirection() * 10.0f)
			* glm::toMat4(camera.GetOrientation())
			* glm::scale(kIdentityMatrix, glm::vec3(1.0f)/*{ 9.53f, 5.36f, 1.0f }*/);

		if (Renderer2D::BeginScene(camera))
		{
			Renderer2D::DrawQuad(transform, Color::Cyan);

			Renderer2D::EndScene();
		}

		//if (Renderer2D::BeginScene(camera))
		//{
		//	DrawSpriteRenderComponent(camera.GetPosition());
		//
		//	Renderer2D::EndScene();
		//}
	}

	void Scene::DrawSpriteRenderComponent(const glm::vec3& cameraPosition)
	{
		auto group = _registry.group<TransformComponent>(entt::get<SpriteRendererComponent>);

		group.sort<TransformComponent>([&](const auto& lhs, const auto& rhs)
		{
			auto lhsDistance = glm::distance(lhs.Position, cameraPosition);
			auto rhsDistance = glm::distance(rhs.Position, cameraPosition);
			return lhsDistance > rhsDistance;
		});

		for (const auto entity : group)
		{
			auto [transform, sprite] = group.get<TransformComponent, SpriteRendererComponent>(entity);
			Renderer2D::DrawSprite(transform.GetTransformMatrix(), sprite, (int)entity);
		}
	}

	void Scene::OnViewportResize(uint32_t width, uint32_t height)
	{
		_viewportWidth = width;
		_viewportHeight = height;

		// Resize our non-FixedAspectRation cameras
		auto view = _registry.view<CameraComponent>();
		for (const auto entity : view)
		{
			auto& cameraComponent = view.get<CameraComponent>(entity);
			if (!cameraComponent.IsFixedAspectRatio || cameraComponent.Camera.GetProjectionType() == SceneCamera::ProjectionType::Perspective)
			{
				cameraComponent.Camera.SetViewportSize(width, height);
			}
		}
	}

	Entity Scene::GetPrimaryCameraEntity()
	{
		auto view = _registry.view<CameraComponent>();
		for (const auto entity : view)
		{
			const auto& camera = view.get<CameraComponent>(entity);
			if (camera.IsPrimary)
			{
				return { entity, this };
			}
		}

		return Entity();
	}

	template<typename T>
	void Scene::OnComponentAdded(Entity entity, T& component)
	{
		static_assert(false);
	}

	template<>
	void Scene::OnComponentAdded<BaseComponent>(Entity entity, BaseComponent& component)
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
		if (_viewportWidth > 0 && _viewportHeight > 0)
		{
			component.Camera.SetViewportSize(_viewportWidth, _viewportHeight);
		}
	}

	template<>
	void Scene::OnComponentAdded<NativeScriptComponent>(Entity entity, NativeScriptComponent& component)
	{}
}
