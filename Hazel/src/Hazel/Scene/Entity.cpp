#include "hzpch.h"
#include "Entity.h"
#include "Components.h"

namespace Hazel
{
	Entity::Entity(entt::entity handle, Scene* scene)
		: _entityHandle(handle), _scene(scene)
	{}

	std::string& Entity::Name()
	{
		return GetComponent<BaseComponent>().Name;
	}

	int& Entity::Tag()
	{
		return GetComponent<BaseComponent>().Tag;
	}

	int& Entity::Layer()
	{
		return GetComponent<BaseComponent>().Layer;
	}

	FamilyComponent& Entity::Family()
	{
		return GetComponent<FamilyComponent>();
	}

	TransformComponent& Entity::Transform()
	{
		return GetComponent<TransformComponent>();
	}
}
