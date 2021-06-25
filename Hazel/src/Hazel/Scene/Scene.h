#pragma once
#include "entt.hpp"
#include "Hazel/Core/Timestep.h"

namespace Hazel
{
	class Entity;

	class Scene
	{
	public:
		Scene() = default;
		~Scene();

		Entity CreateEntity(std::string name = std::string());
		void DestroyEntity(Entity entity);

		void OnUpdate(Timestep timestep);
		void OnViewportResize(uint32_t  width, uint32_t height);

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

		friend class Entity;
		friend class SceneSerializer;
		friend class SceneHierarchyPanel;
	};
}
