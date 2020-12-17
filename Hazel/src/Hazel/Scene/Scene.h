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

	private:
		entt::registry _registry;

		friend class Entity;
	};
}
