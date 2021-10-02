#include "hzpch.h"
#include "Scene.h"
#include "components.h"
#include "ScriptableEntity.h"
#include "Hazel/Renderer/Renderer2D.h"

#include "box2d/b2_world.h"
#include "box2d/b2_body.h"
#include "box2d/b2_fixture.h"
#include "box2d/b2_polygon_shape.h"

namespace Hazel
{
	static b2BodyType Rigidbody2DTypeToBox2DBody(Rigidbody2DComponent::BodyType bodyType)
	{
		switch (bodyType)
		{
		case Hazel::Rigidbody2DComponent::BodyType::Static: return b2_staticBody;
		case Hazel::Rigidbody2DComponent::BodyType::Dynamic: return b2_dynamicBody;
		case Hazel::Rigidbody2DComponent::BodyType::Kinematic: return b2_kinematicBody;
		}

		HZ_CORE_ASSERT(false, "Unknown body Type!");
		return b2_staticBody;
	}


	Scene::~Scene()
	{
		_registry.clear();
	}

	template<typename Component>
	static void CopyComponent(entt::registry& dst, entt::registry& src, const std::unordered_map<UUID, entt::entity>& enttMap)
	{
		src.view<Component>().each([&](const auto entt, Component& component)
		{
			UUID uuid = src.get<IDComponent>(entt).ID;
			HZ_CORE_ASSERT(enttMap.find(uuid) != enttMap.end(), "UUID not found in enttMap!");
			auto dstEnttID = enttMap.at(uuid);

			dst.emplace_or_replace<Component>(dstEnttID, component);
		});
	}

	Ref<Scene> Scene::Copy(Ref<Scene> other)
	{
		Ref<Scene> newScene = CreateRef<Scene>();

		newScene->_viewportWidth = other->_viewportWidth;
		newScene->_viewportHeight = other->_viewportHeight;


		auto& srcSceneRegistry = other->_registry;
		auto& dstSceneRegistry = newScene->_registry;
		std::unordered_map<UUID, entt::entity> enttMap;

		srcSceneRegistry.view<IDComponent>().each([&](const auto entt, IDComponent& idComponent)
		{
			UUID uuid = idComponent.ID;

			const BaseComponent& baseComponent = srcSceneRegistry.get<BaseComponent>(entt);
			const auto& name = baseComponent.Name;
			const auto& tag = baseComponent.Tag;
			const auto& layer = baseComponent.Layer;

			enttMap[uuid] = (entt::entity)newScene->CreateEntityWithUUID(uuid, name, tag, layer);
		});

		// Copy components
		CopyComponent<TransformComponent>(dstSceneRegistry, srcSceneRegistry, enttMap);
		CopyComponent<SpriteRendererComponent>(dstSceneRegistry, srcSceneRegistry, enttMap);
		CopyComponent<CameraComponent>(dstSceneRegistry, srcSceneRegistry, enttMap);
		CopyComponent<NativeScriptComponent>(dstSceneRegistry, srcSceneRegistry, enttMap);
		CopyComponent<Rigidbody2DComponent>(dstSceneRegistry, srcSceneRegistry, enttMap);
		CopyComponent<BoxCollider2DComponent>(dstSceneRegistry, srcSceneRegistry, enttMap);

		return newScene;
	}

	Entity Scene::CreateEntity(const std::string& name, int tag, int layer)
	{
		return CreateEntityWithUUID(UUID(), name, tag, layer);
	}

	Entity Scene::CreateEntityWithUUID(UUID uuid, const std::string& name, int tag, int layer)
	{
		Entity entity = { _registry.create(), this };

		entity.AddComponent<IDComponent>(uuid);

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

	void Scene::OnRuntimeStart()
	{
		_physicsWorld = new b2World({ 0.0f, -9.8f });
		_registry.view<Rigidbody2DComponent>().each([=](const auto entt, Rigidbody2DComponent& rb2d)
		{
			Entity entity = { entt, this };
			auto& transform = entity.Transform();

			b2BodyDef bodyDef;
			bodyDef.type = Rigidbody2DTypeToBox2DBody(rb2d.Type);
			bodyDef.position.Set(transform.Position.x, transform.Position.y);
			bodyDef.angle = transform.Rotation.z;

			b2Body* body = _physicsWorld->CreateBody(&bodyDef);
			body->SetFixedRotation(rb2d.IsFixedRotation);
			rb2d.RuntimeBody = body;

			if (auto* bc2d = entity.TryGetComponent<BoxCollider2DComponent>(); bc2d != nullptr)
			{
				b2PolygonShape boxShape;
				boxShape.SetAsBox(bc2d->Size.x * transform.Scale.x, bc2d->Size.y * transform.Scale.y);

				b2FixtureDef fixtureDef;
				fixtureDef.shape = &boxShape;
				fixtureDef.density = bc2d->Density;
				fixtureDef.friction = bc2d->Friction;
				fixtureDef.restitution = bc2d->Restitution;
				fixtureDef.restitutionThreshold = bc2d->RestitutionThreshold;

				body->CreateFixture(&fixtureDef);
			}
		});
	}

	void Scene::OnRuntimeStop()
	{
		delete _physicsWorld;
		_physicsWorld = nullptr;
	}

	void Scene::OnUpdateRuntime()
	{
		// Update Scripts
		_registry.view<NativeScriptComponent>().each([=](const auto entt, auto& nsc)
		{
			auto& instance = nsc.Instance;

			// TODO Move to OnScenePlay
			if (instance == nullptr)
			{
				instance = nsc.InstantiateScript();
				instance->_entity = { entt, this };
				instance->OnCreate();
			}

			if (!instance->IsEnable)
			{
				return;
			}

			instance->OnUpdate();
		});

		// Physics
		{
			const int32_t velocityInteration = 6;
			const int32_t positionInteration = 2;
			_physicsWorld->Step(Time::GetTimestep(), velocityInteration, positionInteration);

			// Retrieve transform from Box2D
			_registry.view<Rigidbody2DComponent>().each([&](const auto entt, Rigidbody2DComponent& rb2d)
			{
				Entity entity = { entt, this };
				auto& transform = entity.Transform();

				if (auto* bc2d = entity.TryGetComponent<BoxCollider2DComponent>(); bc2d != nullptr)
				{
					auto* body = (b2Body*)rb2d.RuntimeBody;
					const auto& position = body->GetPosition();
					transform.Position.x = position.x;
					transform.Position.y = position.y;
					transform.Rotation.z = body->GetAngle();
				}
			});
		}

		// Render 2D
		Camera* mainCamera = nullptr;
		glm::mat4 cameraTransform;
		glm::vec3 cameraPosition;
		_registry.view<CameraComponent, TransformComponent>().each([&](const auto entity, CameraComponent& camera, TransformComponent& transform)
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
		if (Renderer2D::BeginScene(camera))
		{
			DrawSpriteRenderComponent(camera.GetPosition());

			Renderer2D::EndScene();
		}
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
	void Scene::OnComponentAdded<IDComponent>(Entity entity, IDComponent& component)
	{}

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

	template<>
	void Scene::OnComponentAdded<Rigidbody2DComponent>(Entity entity, Rigidbody2DComponent& component)
	{}

	template<>
	void Scene::OnComponentAdded<BoxCollider2DComponent>(Entity entity, BoxCollider2DComponent& component)
	{}
}
