#include "hzpch.h"
#include "Entity.h"

namespace Hazel
{
	const Entity Entity::Null = { entt::null, nullptr };

	Entity::Entity(entt::entity handle, Scene* scene)
		:_entityHandle(handle), _scene(scene)
	{}
}
