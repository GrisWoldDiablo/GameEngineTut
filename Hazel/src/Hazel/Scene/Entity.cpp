#include "hzpch.h"
#include "Entity.h"
#include "Components.h"

namespace Hazel
{
	Entity::Entity(entt::entity handle, Scene* scene)
		: _entityHandle(handle), _scene(scene) {}

	std::string& Entity::Name() const
	{
		return GetComponent<BaseComponent>().Name;
	}

	int& Entity::Tag() const
	{
		return GetComponent<BaseComponent>().Tag;
	}

	int& Entity::Layer() const
	{
		return GetComponent<BaseComponent>().Layer;
	}

	FamilyComponent& Entity::Family() const
	{
		return GetComponent<FamilyComponent>();
	}

	TransformComponent& Entity::Transform() const
	{
		return GetComponent<TransformComponent>();
	}
}
