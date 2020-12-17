#include "hzpch.h"
#include "Scene.h"
#include "components.h"
#include "Hazel/Renderer/Renderer2D.h"

#include "Entity.h"

namespace Hazel
{
	Scene::Scene()
	{
		
	}

	Scene::~Scene()
	{
		_registry.clear();
	}

	Entity Scene::CreateEntity(std::string name)
	{
		Entity entity = { _registry.create(), this };

		entity.AddComponent<TransformComponent>();

		auto& tagComponent = entity.AddComponent<TagComponent>();
		tagComponent.Tag = name.empty() ? "Entity" : std::move(name);

		return entity;
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