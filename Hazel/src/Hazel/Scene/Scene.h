#pragma once
#include "Hazel/Core/Timestep.h"
#include "Hazel/Core/UUID.h"
#include "Hazel/Renderer/EditorCamera.h"
#include "Hazel/Renderer/Texture.h"
#include "Hazel/Scene/Components.h"

#include "entt.hpp"

class b2World;

namespace Hazel
{
	class Entity;

	class Scene
	{
	public:
		Scene();
		Scene(const std::string& name);
		~Scene();

		static Ref<Scene> Copy(const Ref<Scene>& other);

		Entity CreateEntity(const std::string& name = "Entity", int tag = 0, int layer = 0);
		Entity CreateEntityWithUUID(UUID uuid, const std::string& name = "Entity", int tag = 0, int layer = 0);
		void DestroyEntity(Entity entity);
		bool CheckEntityValidity(const entt::entity entity) const;

		void OnRuntimeStart();
		void OnRuntimeStop();
		void OnSimulationStart();
		void OnSimulationStop();

		void OnUpdateRuntime(Timestep timestep);
		void OnUpdateSimulation(Timestep timestep, const EditorCamera& camera);
		void OnUpdateEditor(Timestep timestep, const EditorCamera& camera);
		void Step(int frames = 1);

		void DrawSpriteRenderComponent(const glm::vec3& cameraPosition);
		void DrawCircleRenderComponent(const glm::vec3& cameraPosition);
		void DrawAudioComponent(const glm::vec3& cameraPosition, bool isRuntime = false);

		void OnViewportResize(uint32_t width, uint32_t height);

		void ReparentEntity(Entity newParent, Entity newChild);
		bool IsChildOf(Entity parent, Entity child);

		Entity DuplicateEntity(Entity entity);
		Entity GetEntityByUUID(UUID uuid);
		Entity GetEntityByName(const std::string& name);

		std::string GetName() const { return _name; }
		void SetName(const std::string& name) { _name = name; }

		Entity GetPrimaryCameraEntity();

		Entity GetRootEntity();
		auto GetEntities() { return _registry.view<IDComponent>(entt::exclude<Root>); }
		template<typename... Components>
		auto GetEntitiesViewWith() { return _registry.view<Components...>(entt::exclude<Root>); }
		template<typename... Components>
		auto GetEntitiesGroupWith() { return _registry.group<Components...>(entt::exclude<Root>); }

		bool IsRunning() const { return _isRunning; }
		bool IsPaused() const { return _isPaused; }
		void SetPause(bool isPaused) { _isPaused = isPaused; }

		bool& ShouldCloneAudioSource() { return _shouldCloneAudioSource; }

		glm::ivec2 GetViewportSize() const { return glm::ivec2(_viewportWidth, _viewportHeight); }

	private:
		template<typename T>
		void OnComponentAdded(Entity entity, T& component);
		template<typename T>
		void OnComponentRemoved(Entity entity, T& component);

		void OnEntityDestroy(Entity entity);

		void OnPhysic2DStart();
		void OnPhysic2DStop();

		void RenderScene(const EditorCamera& camera);

		bool IsChildOfImpl(Entity child, Entity entity);

	private:
		std::string _name;
		bool _isRunning = false;
		bool _isPaused = false;
		int _stepFrames = 0;
		uint32_t _viewportWidth = 0;
		uint32_t _viewportHeight = 0;

		bool _shouldCloneAudioSource = false;

		entt::entity _rootEntt;
		entt::registry _registry;
		std::unordered_map<UUID, entt::entity> _entityMap;

		b2World* _physicsWorld = nullptr;

		static Ref<Texture2D> _sAudioSourceIcon;
		static Ref<Texture2D> _sAudioListenerIcon;

		friend class Entity;
	};
}
