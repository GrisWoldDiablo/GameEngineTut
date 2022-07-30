#include "hzpch.h"
#include "Scene.h"
#include "Components.h"
#include "ScriptableEntity.h"
#include "Hazel/Renderer/Renderer2D.h"
#include "Hazel/Scripting/ScriptEngine.h"

#include "box2d/b2_world.h"
#include "box2d/b2_body.h"
#include "box2d/b2_fixture.h"
#include "box2d/b2_polygon_shape.h"
#include "box2d/b2_circle_shape.h"
#include "Hazel/Core/Random.h"

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


	Scene::Scene(const std::string& name)
		:_name(name)
	{}

	Scene::~Scene()
	{
		_registry.clear();
		delete _physicsWorld;
	}

	template<typename... Component>
	static void CopyComponent(entt::registry& dst, const entt::registry& src, const std::unordered_map<UUID, entt::entity>& enttMap)
	{
		([&]()
		{
			auto view = src.view<Component>();
			for (auto srcEntity : view)
			{
				entt::entity dstEntity = enttMap.at(src.get<IDComponent>(srcEntity).ID);
				auto& srcComponent = src.get<Component>(srcEntity);
				dst.emplace_or_replace<Component>(dstEntity, srcComponent);

			}
		}(), ...);
	}

	template<typename... Component>
	static void CopyComponents(ComponentGroup<Component...>, entt::registry& dst, const entt::registry& src, const std::unordered_map<UUID, entt::entity>& enttMap)
	{
		CopyComponent<Component...>(dst, src, enttMap);
	}


	template<typename... Component>
	static void CopyComponentIfExist(Entity dst, Entity src)
	{
		([&]()
		{
			if (src.HasComponent<Component>())
			{
				dst.AddOrReplaceComponent<Component>(src.GetComponent<Component>());
			}
		}(), ...);
	}

	template<typename... Component>
	static void CopyComponentsIfExist(ComponentGroup<Component...>, Entity dst, Entity src)
	{
		CopyComponentIfExist<Component...>(dst, src);
	}

	Ref<Scene> Scene::Copy(const Ref<Scene>& other)
	{
		Ref<Scene> newScene = CreateRef<Scene>(other->_name);

		newScene->_viewportWidth = other->_viewportWidth;
		newScene->_viewportHeight = other->_viewportHeight;


		auto& srcSceneRegistry = other->_registry;
		auto& dstSceneRegistry = newScene->_registry;
		std::unordered_map<UUID, entt::entity> enttMap;

		auto group = srcSceneRegistry.group<IDComponent, BaseComponent>();

		// Need to run reverse to keep order of entt ID intact.
		std::for_each(group.rbegin(), group.rend(), [&](auto entt)
		{
			const auto [idComponent, baseComponent] = group.get<IDComponent, BaseComponent>(entt);
			UUID uuid = idComponent.ID;

			const auto& name = baseComponent.Name;
			const auto& tag = baseComponent.Tag;
			const auto& layer = baseComponent.Layer;

			enttMap[uuid] = (entt::entity)newScene->CreateEntityWithUUID(uuid, name, tag, layer);
		});

		// Copy components
		CopyComponents(AllComponents{}, dstSceneRegistry, srcSceneRegistry, enttMap);

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

		_entityMap[uuid] = entity;

		return entity;
	}

	void Scene::DestroyEntity(Entity entity)
	{
		_registry.destroy(entity);
		_entityMap.erase(entity.GetUUID());
	}

	bool Scene::CheckEntityValidity(const entt::entity& entity) const
	{
		return _registry.valid(entity);
	}

	void Scene::OnRuntimeStart()
	{
		OnPhysic2DStart();

		// Scripting
		{
			ScriptEngine::OnRuntimeStart(this);
			// Instanciate all script entities
			_registry.view<ScriptComponent>().each([&](const auto entt, const ScriptComponent& scriptComponent)
			{
				if (ScriptEngine::EntityClassExist(scriptComponent.ClassName))
				{
					Entity entity = { entt, this };
					ScriptEngine::OnCreateEntity(entity);
				}
			});
		}
	}

	void Scene::OnRuntimeStop()
	{
		OnPhysic2DStop();

		ScriptEngine::OnRuntimeStop();
	}

	void Scene::OnSimulationStart()
	{
		OnPhysic2DStart();
	}

	void Scene::OnSimulationStop()
	{
		OnPhysic2DStop();
	}

	void Scene::OnUpdateRuntime(Timestep timestep)
	{
		// C# OnUpdate Script
		_registry.view<ScriptComponent>().each([&](const auto entt, const ScriptComponent& scriptComponent)
		{
			if (ScriptEngine::EntityClassExist(scriptComponent.ClassName))
			{
				Entity entity = { entt, this };
				ScriptEngine::OnUpdateEntity(entity, timestep);
			}
		});

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

			instance->OnUpdate(timestep);
		});

		// Physics
		{
			constexpr int32_t velocityInteration = 6;
			constexpr int32_t positionInteration = 2;
			_physicsWorld->Step(timestep, velocityInteration, positionInteration);

			// Retrieve transform from Box2D
			_registry.view<Rigidbody2DComponent>().each([&](const auto entt, const Rigidbody2DComponent& rb2d)
			{
				Entity entity = { entt, this };
				auto& transform = entity.Transform();

				if (entity.HasComponent<BoxCollider2DComponent>()
				  || entity.HasComponent<CircleCollider2DComponent>())
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
		_registry.view<CameraComponent, TransformComponent>().each([&](const auto entity, CameraComponent& camera, const TransformComponent& transform)
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
			DrawCircleRenderComponent(cameraPosition);

			Renderer2D::EndScene();
		}
	}

	void Scene::OnUpdateSimulation(Timestep timestep, const EditorCamera& camera)
	{

		// Physics
		{
			constexpr int32_t kVelocityInteration = 6;
			constexpr int32_t kPositionInteration = 2;
			_physicsWorld->Step(timestep, kVelocityInteration, kPositionInteration);

			// Retrieve transform from Box2D
			_registry.view<Rigidbody2DComponent>().each([&](const auto entt, const Rigidbody2DComponent& rb2d)
			{
				Entity entity = { entt, this };
				auto& transform = entity.Transform();

				if (entity.HasComponent<BoxCollider2DComponent>()
				  || entity.HasComponent<CircleCollider2DComponent>())
				{
					auto* body = (b2Body*)rb2d.RuntimeBody;
					const auto& position = body->GetPosition();
					transform.Position.x = position.x;
					transform.Position.y = position.y;
					transform.Rotation.z = body->GetAngle();
				}
			});
		}


		RenderScene(camera);
	}

	void Scene::OnUpdateEditor(Timestep timestep, const EditorCamera& camera)
	{
		RenderScene(camera);
	}

	void Scene::DrawSpriteRenderComponent(const glm::vec3& cameraPosition)
	{
		auto view = _registry.view<TransformComponent, SpriteRendererComponent>();

		//TODO Fix sorting. Can't have 2 groups sprite and circle render
		//group.sort<TransformComponent>([&](const auto& lhs, const auto& rhs)
		//{
		//	auto lhsDistance = glm::distance(lhs.Position, cameraPosition);
		//	auto rhsDistance = glm::distance(rhs.Position, cameraPosition);
		//	return lhsDistance > rhsDistance;
		//});

		for (const auto entity : view)
		{
			auto [transform, sprite] = view.get<TransformComponent, SpriteRendererComponent>(entity);
			Renderer2D::DrawSprite(transform.GetTransformMatrix(), sprite, (int)entity);

			// Comment out to draw the sprite bounding box for testing.
			// Renderer2D::DrawRect(transform.GetTransformMatrix(), Color::Green, (int)entity);
		}
	}

	void Scene::DrawCircleRenderComponent(const glm::vec3& cameraPosition)
	{
		auto view = _registry.view<TransformComponent, CircleRendererComponent>();

		//TODO Fix sorting.
		//group.sort<TransformComponent>([&](const auto& lhs, const auto& rhs)
		//{
		//	auto lhsDistance = glm::distance(lhs.Position, cameraPosition);
		//	auto rhsDistance = glm::distance(rhs.Position, cameraPosition);
		//	return lhsDistance > rhsDistance;
		//});

		for (const auto entity : view)
		{
			auto [transform, circle] = view.get<TransformComponent, CircleRendererComponent>(entity);
			Renderer2D::DrawCircle(transform.GetTransformMatrix(), circle.Color, circle.Thickness, circle.Fade, (int)entity);
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

	Entity Scene::DuplicateEntity(Entity entity)
	{
		Entity newEntity = CreateEntity(entity.Name());
		CopyComponentsIfExist(AllComponents{}, newEntity, entity);

		return newEntity;
	}

	Entity Scene::GetEntityByUUID(UUID uuid)
	{
		// Maybe assert?
		if (_entityMap.find(uuid) != _entityMap.end())
		{
			return { _entityMap.at(uuid), this };
		}

		return {};
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

		return {};
	}

	void Scene::OnPhysic2DStart()
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

			auto createFixture = [&](const b2Shape& shape, float density, float friction, float restitution, float restitutionThreshold)
			{
				b2FixtureDef fixtureDef;
				fixtureDef.shape = &shape;
				fixtureDef.density = density;
				fixtureDef.friction = friction;
				fixtureDef.restitution = restitution;
				fixtureDef.restitutionThreshold = restitutionThreshold;

				body->CreateFixture(&fixtureDef);
			};

			if (entity.HasComponent<BoxCollider2DComponent>())
			{
				const auto& bc2d = entity.GetComponent<BoxCollider2DComponent>();
				b2PolygonShape boxShape;
				boxShape.SetAsBox
				(
					bc2d.Size.x * transform.Scale.x,
					bc2d.Size.y * transform.Scale.y,
					{ bc2d.Offset.x, bc2d.Offset.y },
					glm::radians(bc2d.Rotation)
				);


				createFixture(boxShape, bc2d.Density, bc2d.Friction, bc2d.Restitution, bc2d.RestitutionThreshold);
			}

			if (entity.HasComponent<CircleCollider2DComponent>())
			{
				const auto& cc2d = entity.GetComponent<CircleCollider2DComponent>();
				b2CircleShape circleShape;
				circleShape.m_p.Set(cc2d.Offset.x, cc2d.Offset.y);
				circleShape.m_radius = cc2d.Radius * glm::max(transform.Scale.x, transform.Scale.y);

				createFixture(circleShape, cc2d.Density, cc2d.Friction, cc2d.Restitution, cc2d.RestitutionThreshold);
			}
		});
	}

	void Scene::OnPhysic2DStop()
	{
		delete _physicsWorld;
		_physicsWorld = nullptr;
	}

	void Scene::RenderScene(const EditorCamera& camera)
	{
		if (Renderer2D::BeginScene(camera))
		{
			DrawSpriteRenderComponent(camera.GetPosition());
			DrawCircleRenderComponent(camera.GetPosition());

			// TESTING
			//Renderer2D::DrawLine(glm::vec3(Random::Float()), glm::vec3(5.0f), Color::Magenta);
			//Renderer2D::DrawRect(glm::vec3(0.0f), glm::vec3(1.0f), Color::White);

			Renderer2D::EndScene();
		}
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
	void Scene::OnComponentAdded<CircleRendererComponent>(Entity entity, CircleRendererComponent& component)
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
	void Scene::OnComponentAdded<ScriptComponent>(Entity entity, ScriptComponent& component)
	{}

	template<>
	void Scene::OnComponentAdded<NativeScriptComponent>(Entity entity, NativeScriptComponent& component)
	{}

	template<>
	void Scene::OnComponentAdded<Rigidbody2DComponent>(Entity entity, Rigidbody2DComponent& component)
	{}

	template<>
	void Scene::OnComponentAdded<BoxCollider2DComponent>(Entity entity, BoxCollider2DComponent& component)
	{}

	template<>
	void Scene::OnComponentAdded<CircleCollider2DComponent>(Entity entity, CircleCollider2DComponent& component)
	{}
}
