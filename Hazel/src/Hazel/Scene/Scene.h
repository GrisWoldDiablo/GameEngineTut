#pragma once
#include "Hazel/Core/Timestep.h"
#include "Hazel/Core/UUID.h"
#include "Hazel/Renderer/EditorCamera.h"

#include "entt.hpp"

class b2World;

namespace Hazel
{
	class Entity;

	class Scene
	{
	public:
		Scene() = default;
		Scene(const std::string& name);
		~Scene();

		static Ref<Scene> Copy(Ref<Scene> scene);

		Entity CreateEntity(const std::string& name = "Entity", int tag = 0, int layer = 0);
		Entity CreateEntityWithUUID(UUID uuid, const std::string& name = "Entity", int tag = 0, int layer = 0);
		void DestroyEntity(Entity entity);
		const bool CheckEntityValidity(const entt::entity entity);

		void OnRuntimeStart();
		void OnRuntimeStop();

		void OnUpdateRuntime();
		void OnUpdateEditor(EditorCamera& camera);

		void DrawSpriteRenderComponent(const glm::vec3& cameraPosition);
		void DrawCircleRenderComponent(const glm::vec3& cameraPosition);

		void OnViewportResize(uint32_t  width, uint32_t height);

		Entity DuplicateEntity(Entity entity);

		std::string GetName() const { return _name; }
		void SetName(const std::string& name) { _name = name; }

		Entity GetPrimaryCameraEntity();

	private:
		template<typename T>
		void OnComponentAdded(Entity entity, T& component);

	private:
		entt::registry _registry;
		uint32_t _viewportWidth = 0;
		uint32_t _viewportHeight = 0;
		std::string _name;

		b2World* _physicsWorld = nullptr;

		friend class Entity;
		friend class SceneSerializer;
		friend class SceneHierarchyPanel;
	};
}
