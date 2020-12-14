#include "hzpch.h"
#include "Scene.h"

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
		struct MeshComponent
		{
			int Mesh;
			MeshComponent() = default;
		};

		struct TransformComponent
		{
			glm::mat4 Transform;

			TransformComponent() = default;
			TransformComponent(const TransformComponent&) = default;
			TransformComponent(const glm::mat4& transform)
				:Transform(transform)
			{
			}

			operator glm::mat4& () { return Transform; }
			operator const glm::mat4& () const { return Transform; }
		};

		auto mainEntity = _registry.create();
		_registry.on_construct<TransformComponent>().connect<&OnTransformContruct>();
		_registry.emplace<TransformComponent>(mainEntity, glm::mat4(1.0f));

		if (_registry.has<TransformComponent>(mainEntity))
		{
			TransformComponent& transform = _registry.get<TransformComponent>(mainEntity);
		}

		auto view = _registry.view<TransformComponent>();
		for (auto entity : view)
		{
			TransformComponent& transform = view.get<TransformComponent>(entity);
		}

		auto group = _registry.group<TransformComponent>(entt::get<MeshComponent>);
		for (auto entity : group)
		{
			auto[transform, mesh] = group.get<TransformComponent, MeshComponent>(entity);
			transform.Transform = glm::mat4(2.0f);
			mesh.Mesh = 2;
		}
	}

	Scene::~Scene()
	{

	}

}