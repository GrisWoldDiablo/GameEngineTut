#pragma once
#include "Hazel/Core/Timestep.h"
#include "Hazel/Core/UUID.h"
#include "Hazel/Renderer/EditorCamera.h"
#include "Hazel/Renderer/Texture.h"

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
		bool CheckEntityValidity(const entt::entity& entity) const;

		void OnRuntimeStart();
		void OnRuntimeStop();
		void OnSimulationStart();
		void OnSimulationStop();

		void OnUpdateRuntime(Timestep timestep);
		void OnUpdateSimulation(Timestep timestep, const EditorCamera& camera);
		void OnUpdateEditor(Timestep timestep, const EditorCamera& camera);

		void DrawSpriteRenderComponent(const glm::vec3& cameraPosition);
		void DrawCircleRenderComponent(const glm::vec3& cameraPosition);
		void DrawAudioComponent(const glm::vec3& cameraPosition, bool isRuntime = false);

		void OnViewportResize(uint32_t  width, uint32_t height);

		Entity DuplicateEntity(Entity entity);
		Entity GetEntityByUUID(UUID uuid);
		Entity GetEntityByName(const std::string& name);

		std::string GetName() const { return _name; }
		void SetName(const std::string& name) { _name = name; }

		Entity GetPrimaryCameraEntity();

		template<typename... Components>
		auto GetAllEntitiesWith() { return _registry.view<Components...>(); }

		bool IsRunning() const { return _isRunning; }

	private:
		template<typename T>
		void OnComponentAdded(Entity entity, T& component);
		template<typename T>
		void OnComponentRemoved(Entity entity, T& component);

		void OnEntityDestroy(Entity entity);

		void OnPhysic2DStart();
		void OnPhysic2DStop();

		void RenderScene(const EditorCamera& camera);

	private:
		entt::registry _registry;
		uint32_t _viewportWidth = 0;
		uint32_t _viewportHeight = 0;
		std::string _name;

		b2World* _physicsWorld = nullptr;
		bool _isRunning = false;

		std::unordered_map<UUID, entt::entity> _entityMap;

		static Ref<Texture2D> _sAudioSourceIcon;
		static Ref<Texture2D> _sAudioListenerIcon;

		friend class Entity;
		friend class SceneSerializer;
		friend class SceneHierarchyPanel;
	};
}
