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

		void OnUpdate(Timestep timestep);
		void OnViewportResize(uint32_t  width, uint32_t height);

	private:
		void SortSpriteRendererGroup(bool forced = false);

	private:
		entt::registry _registry;
		uint32_t _viewportWidth = 0;
		uint32_t _viewportHeight = 0;
		uint32_t _spriteAmount = 0;
		friend class Entity;
		friend class SceneHierarchyPanel;
	};
}
