#include "hzpch.h"
#include "Scene.h"
#include "Components.h"
#include "ScriptableEntity.h"

#include "Hazel/Core/Random.h"
#include "Hazel/Renderer/Renderer2D.h"
#include "Hazel/Scripting/ScriptEngine.h"
#include "Hazel/Audio/AudioEngine.h"

#include "box2d/b2_world.h"
#include "box2d/b2_body.h"
#include "box2d/b2_fixture.h"
#include "box2d/b2_polygon_shape.h"
#include "box2d/b2_circle_shape.h"

namespace Hazel
{
	Ref<Texture2D> Scene::_sAudioSourceIcon;
	Ref<Texture2D> Scene::_sAudioListenerIcon;

	Scene::Scene()
	{
		// TODO create an Icon Manager.
		if (_sAudioSourceIcon == nullptr)
		{
			_sAudioSourceIcon = Texture2D::Create("Resources/Icons/General/AudioSource256.png");
			_sAudioListenerIcon = Texture2D::Create("Resources/Icons/General/AudioListener256.png");
		}

		_rootEntt = _registry.create();
		Entity rootEntity = {_rootEntt, this};
		_registry.emplace<Root>(_rootEntt);

		auto uuid = UUID(69420);
		rootEntity.AddComponent<IDComponent>(uuid);

		rootEntity.AddComponent<FamilyComponent>();

		auto& baseComponent = rootEntity.AddComponent<BaseComponent>();
		baseComponent.Name = "Root";
		baseComponent.Tag = 0;
		baseComponent.Layer = 0;

		rootEntity.AddComponent<TransformComponent>();

		_entityMap[uuid] = rootEntity;
	}

	Scene::Scene(const std::string& name) : Scene()
	{
		_name = name;
	}

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
		newScene->_shouldCloneAudioSource = other->_shouldCloneAudioSource;
		newScene->_isPaused = other->_isPaused;

		auto group = other->GetEntitiesGroupWith<IDComponent, BaseComponent>();
		for (const auto [enttID, idComponent, baseComponent] : group.each())
		{
			const auto uuid = idComponent.ID;

			const auto& name = baseComponent.Name;
			const auto& tag = baseComponent.Tag;
			const auto& layer = baseComponent.Layer;

			newScene->CreateEntityWithUUID(uuid, name, tag, layer);
		}

		auto& srcSceneRegistry = other->_registry;
		auto& dstSceneRegistry = newScene->_registry;

		// Copy components
		CopyComponents(AllComponents{}, dstSceneRegistry, srcSceneRegistry, newScene->_entityMap);
		CopyComponent<FamilyComponent>(dstSceneRegistry, srcSceneRegistry, newScene->_entityMap);

		return newScene;
	}

	Entity Scene::CreateEntity(const std::string& name, int tag, int layer)
	{
		return CreateEntityWithUUID(UUID(), name, tag, layer);
	}

	Entity Scene::CreateEntityWithUUID(UUID uuid, const std::string& name, int tag, int layer)
	{
		Entity entity = {_registry.create(), this};

		entity.AddComponent<IDComponent>(uuid);

		entity.AddComponent<FamilyComponent>();

		auto& baseComponent = entity.AddComponent<BaseComponent>();
		baseComponent.Name = name;
		baseComponent.Tag = tag;
		baseComponent.Layer = layer;

		entity.AddComponent<TransformComponent>();

		_entityMap[uuid] = entity;

		// TODO Add create entity with parent.
		ReparentEntity(GetRootEntity(), entity);

		return entity;
	}

	void Scene::DestroyEntity(Entity entity)
	{
		auto currentEntityID = entity.Family().ChildID;
		while (auto childEntity = GetEntityByUUID(currentEntityID))
		{
			currentEntityID = childEntity.Family().NextSiblingID;
			DestroyEntity(childEntity);
		}

		ReparentEntity(Entity(), entity);

		OnEntityDestroy(entity);
		_entityMap.erase(entity.GetUUID());
		_registry.destroy(entity);
	}

	bool Scene::CheckEntityValidity(const entt::entity entity) const
	{
		return _registry.valid(entity);
	}

	void Scene::OnRuntimeStart()
	{
		_isRunning = true;

		OnPhysic2DStart();

		// Audio
		{
			// TODO? Should we do this here?
			AudioEngine::StopAllAudioSources();

			for (auto&& [enttID, component] : GetEntitiesViewWith<AudioSourceComponent>().each())
			{
				if (component.AudioSource)
				{
					if (_shouldCloneAudioSource)
					{
						component.AudioSource = AudioEngine::CloneAudioSource(component.AudioSource);
					}

					if (component.IsAutoPlay)
					{
						component.AudioSource->Play();
					}
				}
			}
		}

		// Scripting
		{
			ScriptEngine::OnRuntimeStart(this);
			// Instanciate all script entities
			for (auto&& [enttID, component] : GetEntitiesViewWith<ScriptComponent>().each())
			{
				if (ScriptEngine::EntityClassExist(component.ClassName))
				{
					Entity entity = {enttID, this};
					ScriptEngine::OnCreateEntity(entity);
				}
			}
		}
	}

	void Scene::OnRuntimeStop()
	{
		// TODO? Should we do this here?
		AudioEngine::StopAllAudioSources();

		OnPhysic2DStop();

		ScriptEngine::OnRuntimeStop();

		_isRunning = false;
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
		if (!_isPaused || _stepFrames-- > 0)
		{
			// C# OnUpdate Script
			for (auto&& [enttID, component] : GetEntitiesViewWith<ScriptComponent>().each())
			{
				Entity entity = {enttID, this};
				ScriptEngine::OnUpdateEntity(entity, timestep);
			}

			for (auto&& [enttID, component] : GetEntitiesViewWith<NativeScriptComponent>().each())
			{
				auto& instance = component.Instance;

				// TODO Move to OnScenePlay
				if (instance == nullptr)
				{
					instance = component.InstantiateScript();
					instance->_entity = {enttID, this};
					instance->OnCreate();
				}

				if (!instance->IsEnable)
				{
					return;
				}

				instance->OnUpdate(timestep);
			}

			// Physics
			{
				constexpr int32_t velocityInteration = 6;
				constexpr int32_t positionInteration = 2;
				_physicsWorld->Step(timestep, velocityInteration, positionInteration);

				// Retrieve transform from Box2D
				for (auto&& [enttID, component, transform] : GetEntitiesViewWith<Rigidbody2DComponent, TransformComponent>().each())
				{
					Entity entity = {enttID, this};

					if (entity.HasComponent<BoxCollider2DComponent>()
						|| entity.HasComponent<CircleCollider2DComponent>())
					{
						const auto* body = static_cast<b2Body*>(component.RuntimeBody);
						const auto& position = body->GetPosition();
						transform.Position.x = position.x;
						transform.Position.y = position.y;
						transform.Rotation.z = body->GetAngle();
					}
				}
			}
		}

		// Render 2D
		Camera* mainCamera = nullptr;
		glm::mat4 cameraTransform;
		glm::vec3 cameraPosition;
		for (const auto&& [enttID, camera, transform] : GetEntitiesViewWith<CameraComponent, TransformComponent>().each())
		{
			if (camera.IsPrimary)
			{
				mainCamera = &camera.Camera;
				cameraTransform = transform.GetWorldTransformMatrix();
				cameraPosition = transform.Position;
				break;
			}
		}

		if (mainCamera == nullptr)
		{
			return;
		}

		if (Renderer2D::BeginScene(*mainCamera, cameraTransform))
		{
			DrawSpriteRenderComponent(cameraPosition);
			DrawCircleRenderComponent(cameraPosition);
			DrawAudioComponent(cameraPosition, true);

			Renderer2D::EndScene();
		}
	}

	void Scene::OnUpdateSimulation(Timestep timestep, const EditorCamera& camera)
	{
		if (!_isPaused || _stepFrames-- > 0)
		{
			// Physics
			{
				constexpr int32_t kVelocityInteration = 6;
				constexpr int32_t kPositionInteration = 2;
				_physicsWorld->Step(timestep, kVelocityInteration, kPositionInteration);

				// Retrieve transform from Box2D
				for (auto&& [enttID, component, transform] : GetEntitiesViewWith<Rigidbody2DComponent, TransformComponent>().each())
				{
					Entity entity = {enttID, this};

					if (entity.HasComponent<BoxCollider2DComponent>()
						|| entity.HasComponent<CircleCollider2DComponent>())
					{
						const auto* body = static_cast<b2Body*>(component.RuntimeBody);
						const auto& position = body->GetPosition();
						transform.Position.x = position.x;
						transform.Position.y = position.y;
						transform.Rotation.z = body->GetAngle();
					}
				}
			}
		}

		RenderScene(camera);
	}

	void Scene::OnUpdateEditor(Timestep timestep, const EditorCamera& camera)
	{
		RenderScene(camera);
	}

	void Scene::Step(int frames)
	{
		_stepFrames = frames;
	}

	void Scene::DrawSpriteRenderComponent(const glm::vec3& cameraPosition)
	{
		auto entities = GetEntitiesViewWith<SpriteRendererComponent, TransformComponent, FamilyComponent>();
		//TODO Fix sorting?. Can't have 2 groups sprite and circle render
		//group.sort<TransformComponent>([&](const auto& lhs, const auto& rhs)
		//{
		//	auto lhsDistance = glm::distance(lhs.Position, cameraPosition);
		//	auto rhsDistance = glm::distance(rhs.Position, cameraPosition);
		//	return lhsDistance > rhsDistance;
		//});
		for (const auto&& [enttID, sprite, transform] : GetEntitiesViewWith<SpriteRendererComponent, TransformComponent>().each())
		{
			Renderer2D::DrawSprite(transform.GetWorldTransformMatrix(), sprite, (int)enttID);

			// Comment out to draw the sprite bounding box for testing.
			// Renderer2D::DrawRect(transform.GetTransformMatrix(), Color::Green, (int)entity);
		}
	}

	void Scene::DrawCircleRenderComponent(const glm::vec3& cameraPosition)
	{
		for (const auto&& [enttID, circle, transform] : GetEntitiesViewWith<CircleRendererComponent, TransformComponent>().each())
		{
			Renderer2D::DrawCircle(transform.GetWorldTransformMatrix(), circle.Color, circle.Thickness, circle.Fade, (int)enttID);
		}
	}

	void Scene::DrawAudioComponent(const glm::vec3& cameraPosition, bool isRuntime)
	{
		//TODO create draw Icon generic.

		const float cameraPositionZ = cameraPosition.z;
		for (const auto&& [enttID, audioSource, transform] : GetEntitiesViewWith<AudioSourceComponent, TransformComponent>().each())
		{
			if (!isRuntime || audioSource.IsVisibleInGame)
			{
				const float sign = cameraPositionZ > transform.Position.z ? 1.0f : -1.0f;
				auto position = transform.Position;
				position.z += 0.001f * sign;
				auto scale = glm::vec3(1.0f);
				scale.x *= sign;
				Renderer2D::DrawQuad(position, scale, _sAudioSourceIcon, glm::vec2(1.0f), Color::White, static_cast<int>(enttID));
			}
		}

		for (const auto&& [enttID, audioListener, transform] : GetEntitiesViewWith<AudioListenerComponent, TransformComponent>().each())
		{
			if (!isRuntime || audioListener.IsVisibleInGame)
			{
				const float sign = cameraPositionZ > transform.Position.z ? 1.0f : -1.0f;
				auto position = transform.Position;
				position.z += 0.001f * sign;
				auto scale = glm::vec3(1.0f);
				scale.x *= sign;
				Renderer2D::DrawQuad(position, scale, _sAudioListenerIcon, glm::vec2(1.0f), Color::White, static_cast<int>(enttID));
			}
		}
	}

	void Scene::OnViewportResize(uint32_t width, uint32_t height)
	{
		if (_viewportWidth == width && _viewportHeight == height)
		{
			return;
		}

		_viewportWidth = width;
		_viewportHeight = height;

		// Resize our non-FixedAspectRation cameras
		for (const auto&& [enttID, component] : GetEntitiesViewWith<CameraComponent>().each())
		{
			if (!component.IsFixedAspectRatio || component.Camera.GetProjectionType() == SceneCamera::ProjectionType::Perspective)
			{
				component.Camera.SetViewportSize(width, height);
			}
		}
	}

	void Scene::ReparentEntity(Entity newParent, Entity newChild)
	{
		if (IsChildOf(newParent, newChild))
		{
			return;
		}

		auto currentChildID = UUID::Invalid;
		if (newParent)
		{
			if (newParent.GetUUID() == newChild.Family().ParentID)
			{
				return;
			}

			auto& newParentEntityIDs = newParent.Family();

			currentChildID = newParentEntityIDs.ChildID;

			if (newParent && !currentChildID)
			{
				newParentEntityIDs.ChildID = newChild.GetUUID();
			}
		}

		auto& newChildFamily = newChild.Family();
		if (const auto newChildPreviousSibling = GetEntityByUUID(newChildFamily.PreviousSiblingID))
		{
			auto& newChildPreviousSiblingIDs = newChildPreviousSibling.Family();
			newChildPreviousSiblingIDs.NextSiblingID = newChildFamily.NextSiblingID;
		}

		if (const auto newChildNextSibling = GetEntityByUUID(newChildFamily.NextSiblingID))
		{
			auto& newChildNextSiblingIDs = newChildNextSibling.Family();
			newChildNextSiblingIDs.PreviousSiblingID = newChildFamily.PreviousSiblingID;
		}

		if (const auto newChildParent = GetEntityByUUID(newChildFamily.ParentID))
		{
			auto& newChildParentIDs = newChildParent.Family();
			if (newChildParentIDs.ChildID == newChild.GetUUID())
			{
				newChildParentIDs.ChildID = newChildFamily.NextSiblingID;
			}
		}

		newChildFamily.ParentID = UUID::Invalid;
		newChildFamily.PreviousSiblingID = UUID::Invalid;
		newChildFamily.NextSiblingID = UUID::Invalid;

		while (auto newParentChildEntity = GetEntityByUUID(currentChildID))
		{
			auto& newParentChildEntityIDs = newParentChildEntity.Family();
			if (newParentChildEntityIDs.NextSiblingID)
			{
				currentChildID = newParentChildEntityIDs.NextSiblingID;
				continue;
			}

			newParentChildEntityIDs.NextSiblingID = newChild.GetUUID();
			newChildFamily.PreviousSiblingID = newParentChildEntity.GetUUID();
			break;
		}

		auto& newChildTransform = newChild.Transform();
		if (newParent)
		{
			const glm::mat4 currentWorldTransform = newChildTransform.GetWorldTransformMatrix();
			const glm::vec3 scale = newChildTransform.Scale;
			newChildFamily.ParentID = newParent.GetUUID();
			newChildTransform.ParentTransform = &newParent.Transform();
			const glm::mat4 localTransform = glm::inverse(newParent.Transform().GetWorldTransformMatrix()) * currentWorldTransform;
			newChildTransform.SetLocalTransform(localTransform);
			newChildTransform.Scale = scale;
		}
		else
		{
			const glm::mat4 currentWorldTransform = newChildTransform.GetWorldTransformMatrix();
			newChildFamily.ParentID = UUID::Invalid;
			newChildTransform.ParentTransform = nullptr;
			newChildTransform.SetLocalTransform(currentWorldTransform);
		}
	}


	bool Scene::IsChildOf(Entity child, Entity entity)
	{
		return entity && IsChildOfImpl(child, GetEntityByUUID(entity.Family().ChildID));
	}

	bool Scene::IsChildOfImpl(Entity child, Entity entity)
	{
		if (!child || !entity)
		{
			return false;
		}

		if (child.GetUUID() == entity.GetUUID())
		{
			return true;
		}

		auto currentEntityID = entity.Family().ChildID;
		while (auto entityChild = GetEntityByUUID(currentEntityID))
		{
			if (IsChildOfImpl(child, entityChild))
			{
				return true;
			}
			currentEntityID = entityChild.Family().ChildID;
		}

		currentEntityID = entity.Family().NextSiblingID;
		while (auto entityNextSibling = GetEntityByUUID(currentEntityID))
		{
			if (IsChildOfImpl(child, entityNextSibling))
			{
				return true;
			}
			currentEntityID = entityNextSibling.Family().NextSiblingID;
		}

		return false;
	}

	Entity Scene::DuplicateEntity(Entity entity)
	{
		HZ_ASSERT(!_isRunning, "Cannot duplicate while scene is running.");

		const Entity newEntity = CreateEntity(entity.Name());
		CopyComponentsIfExist(AllComponents{}, newEntity, entity);

		ReparentEntity(GetEntityByUUID(entity.Family().ParentID), newEntity);

		return newEntity;
	}

	Entity Scene::GetEntityByUUID(UUID uuid)
	{
		// Maybe assert?
		if (uuid != UUID::Invalid && _entityMap.contains(uuid))
		{
			return {_entityMap.at(uuid), this};
		}

		return {};
	}

	Entity Scene::GetEntityByName(const std::string& name)
	{
		for (const auto&& [enttID, component] : GetEntitiesViewWith<BaseComponent>().each())
		{
			if (component.Name == name)
			{
				return {enttID, this};
			}
		}

		return {};
	}

	Entity Scene::GetPrimaryCameraEntity()
	{
		for (const auto&& [enttID, component] : GetEntitiesViewWith<CameraComponent>().each())
		{
			if (component.IsPrimary)
			{
				return {enttID, this};
			}
		}

		return {};
	}

	Entity Scene::GetRootEntity()
	{
		return {_rootEntt, this};
	}

	void Scene::OnPhysic2DStart()
	{
		_physicsWorld = new b2World({0.0f, -9.8f});

		GetEntitiesViewWith<Rigidbody2DComponent>().each([&](const auto enttID, Rigidbody2DComponent& rb2d)
		{
			Entity entity = {enttID, this};
			const auto& transform = entity.Transform();

			if (entity.Family().ChildID) { }

			b2BodyDef bodyDef;
			bodyDef.type = static_cast<b2BodyType>(Rigidbody2DComponent::TypeToBox2DBody(rb2d.Type));
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
					{bc2d.Offset.x, bc2d.Offset.y},
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

#if 0 // Welding parent child
		for (const auto&& [enttID,rb2d,family] : GetEntitiesViewWith<Rigidbody2DComponent, FamilyComponent>().each())
		{
			if (family.ParentID)
			{
				if (auto parent = this->GetEntityByUUID(family.ParentID))
				{
					if (parent.HasComponent<Rigidbody2DComponent>())
					{
						const auto& parentRb2d = parent.GetComponent<Rigidbody2DComponent>();

						b2WeldJointDef jointDef;
						auto* bodyA = static_cast<b2Body*>(rb2d.RuntimeBody);
						auto* bodyB = static_cast<b2Body*>(parentRb2d.RuntimeBody);
						jointDef.Initialize(bodyA, bodyB, bodyB->GetPosition());
						_physicsWorld->CreateJoint(&jointDef);
					}
				}
			}
		}
#endif
	}

	void Scene::OnPhysic2DStop()
	{
		delete _physicsWorld;
		_physicsWorld = nullptr;
	}

	void Scene::RenderScene(const EditorCamera& camera)
	{
		if (Renderer2D::BeginScene(camera.GetViewProjection()))
		{
			DrawSpriteRenderComponent(camera.GetPosition());
			DrawCircleRenderComponent(camera.GetPosition());
			DrawAudioComponent(camera.GetPosition());

			// TESTING
			//Renderer2D::DrawLine(glm::vec3(Random::Float()), glm::vec3(5.0f), Color::Magenta);
			//Renderer2D::DrawRect(glm::vec3(0.0f), glm::vec3(1.0f), Color::White);

			Renderer2D::EndScene();
		}
	}

#pragma region OnComponentAdded
	template<typename T>
	void Scene::OnComponentAdded(Entity entity, T& component)
	{
		// Commented because of C++20.
		// TODO Find equivalent.
		//static_assert(false);
	}

	template<>
	void Scene::OnComponentAdded<IDComponent>(Entity entity, IDComponent& component) {}

	template<>
	void Scene::OnComponentAdded<BaseComponent>(Entity entity, BaseComponent& component) {}

	template<>
	void Scene::OnComponentAdded<TransformComponent>(Entity entity, TransformComponent& component) {}

	template<>
	void Scene::OnComponentAdded<SpriteRendererComponent>(Entity entity, SpriteRendererComponent& component) {}

	template<>
	void Scene::OnComponentAdded<CircleRendererComponent>(Entity entity, CircleRendererComponent& component) {}

	template<>
	void Scene::OnComponentAdded<CameraComponent>(Entity entity, CameraComponent& component)
	{
		if (_viewportWidth > 0 && _viewportHeight > 0)
		{
			component.Camera.SetViewportSize(_viewportWidth, _viewportHeight);
		}
	}

	template<>
	void Scene::OnComponentAdded<ScriptComponent>(Entity entity, ScriptComponent& component) {}

	template<>
	void Scene::OnComponentAdded<NativeScriptComponent>(Entity entity, NativeScriptComponent& component) {}

	template<>
	void Scene::OnComponentAdded<Rigidbody2DComponent>(Entity entity, Rigidbody2DComponent& component) {}

	template<>
	void Scene::OnComponentAdded<BoxCollider2DComponent>(Entity entity, BoxCollider2DComponent& component) {}

	template<>
	void Scene::OnComponentAdded<CircleCollider2DComponent>(Entity entity, CircleCollider2DComponent& component) {}

	template<>
	void Scene::OnComponentAdded<AudioSourceComponent>(Entity entity, AudioSourceComponent& component)
	{
		if (component.AudioSource)
		{
			const auto& path = component.AudioSource->GetPath();
			component.AudioSource = AudioSource::Create(path);
		}
	}

	template<>
	void Scene::OnComponentAdded<AudioListenerComponent>(Entity entity, AudioListenerComponent& component)
	{
		for (const auto enttID : GetEntitiesViewWith<AudioListenerComponent>())
		{
			if (enttID != entity)
			{
				return;
			}
		}

		AudioEngine::SetListenerPosition(entity.Transform().Position);
	}
#pragma endregion

#pragma region OnComponentRemoved
	template<typename T>
	void Scene::OnComponentRemoved(Entity entity, T& component)
	{
		// Commented because of C++20.
		// TODO Find equivalent.
		//static_assert(false);
	}

	template<>
	void Scene::OnComponentRemoved<IDComponent>(Entity entity, IDComponent& component) {}

	template<>
	void Scene::OnComponentRemoved<BaseComponent>(Entity entity, BaseComponent& component) {}

	template<>
	void Scene::OnComponentRemoved<TransformComponent>(Entity entity, TransformComponent& component) {}

	template<>
	void Scene::OnComponentRemoved<SpriteRendererComponent>(Entity entity, SpriteRendererComponent& component) {}

	template<>
	void Scene::OnComponentRemoved<CircleRendererComponent>(Entity entity, CircleRendererComponent& component) {}

	template<>
	void Scene::OnComponentRemoved<CameraComponent>(Entity entity, CameraComponent& component) {}

	template<>
	void Scene::OnComponentRemoved<ScriptComponent>(Entity entity, ScriptComponent& component)
	{
		// TODO deal with remove at runtime.
	}

	template<>
	void Scene::OnComponentRemoved<NativeScriptComponent>(Entity entity, NativeScriptComponent& component) {}

	template<>
	void Scene::OnComponentRemoved<Rigidbody2DComponent>(Entity entity, Rigidbody2DComponent& component) {}

	template<>
	void Scene::OnComponentRemoved<BoxCollider2DComponent>(Entity entity, BoxCollider2DComponent& component) {}

	template<>
	void Scene::OnComponentRemoved<CircleCollider2DComponent>(Entity entity, CircleCollider2DComponent& component) {}

	template<>
	void Scene::OnComponentRemoved<AudioSourceComponent>(Entity entity, AudioSourceComponent& component)
	{
		if (component.AudioSource)
		{
			component.AudioSource->Stop();
		}
	}

	template<>
	void Scene::OnComponentRemoved<AudioListenerComponent>(Entity entity, AudioListenerComponent& component)
	{
		glm::vec3 position{};

		for (const auto&& [enttID, component, transform] : GetEntitiesViewWith<AudioListenerComponent, TransformComponent>().each())
		{
			if (entity != enttID)
			{
				position = transform.Position;
				break;
			}
		}

		HZ_CORE_LINFO("Audio Listener removed. New Listener position set to {0}", position);
		AudioEngine::SetListenerPosition(position);
	}
#pragma endregion

	template<typename T, typename Function>
	static void CleanUpComponent(Entity entity, Function function)
	{
		if (entity.HasComponent<T>())
		{
			auto& component = entity.GetComponent<T>();
			function(component);
		}
	}

	void Scene::OnEntityDestroy(Entity entity)
	{
		if (!entity)
		{
			return;
		}

		CleanUpComponent<AudioListenerComponent>(entity, [&](AudioListenerComponent& component)
		{
			OnComponentRemoved(entity, component);
		});

		CleanUpComponent<AudioSourceComponent>(entity, [&](AudioSourceComponent& component)
		{
			OnComponentRemoved(entity, component);
		});

		if (_isRunning)
		{
			CleanUpComponent<ScriptComponent>(entity, [&](ScriptComponent& component)
			{
				ScriptEngine::OnDestroyEntity(entity);
			});
		}
	}
}
