#pragma once
#include "entt.hpp"
#include "Hazel/Core/Timestep.h"


namespace Hazel
{
	class Entity;
	
	class Scene
	{
	public:
		Scene();
		~Scene();

		Entity CreateEntity(std::string name = std::string());

		// TEMP
		entt::registry& Reg() { return _registry; }

		void OnUpdate(Timestep timestep);

		void OnViewportResize(uint32_t  width, uint32_t height);
	private:
		entt::registry _registry;
		uint32_t _viewportWidth = 0;
		uint32_t _viewportHeight = 0;

		friend class Entity;
		friend class SceneHierarchyPanel;
	};
}
