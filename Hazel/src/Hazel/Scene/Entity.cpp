#include "hzpch.h"
#include "Entity.h"

namespace Hazel
{
	Entity::Entity(entt::entity handle, Scene* scene)
		:_entityHandle(handle), _scene(scene)
	{
	}
}