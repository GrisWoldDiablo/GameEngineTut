#include "hzpch.h"
#include "Scene.h"
#include "components.h"
#include "Hazel/Renderer/Renderer2D.h"

namespace Hazel
{
	static void DoMaths(const glm::mat4& transform)
	{

	}

	static void OnTransformContruct(entt::registry& registry, entt::entity entity)
	{

	}

	Scene::Scene()
	{
		
	}

	Scene::~Scene()
	{
		_registry.clear();
	}

	entt::entity Scene::CreateEntity()
	{
		return _registry.create();
	}

	void Scene::OnUpdate(Timestep timestep)
	{
		auto group = _registry.group<TransformComponent>(entt::get<SpriteRendererComponent>);
		for (auto entity : group)
		{
			auto& [transform, sprite] =  group.get<TransformComponent, SpriteRendererComponent>(entity);
			Renderer2D::DrawQuad(transform, sprite.Color);
		}
	}
}