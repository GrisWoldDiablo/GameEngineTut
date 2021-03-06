#pragma once
#include "Scene.h"
//#include "Components.h"

namespace Hazel
{
	struct TransformComponent;

	class Entity
	{
	public:
		Entity() = default;
		Entity(entt::entity handle, Scene* scene);
		Entity(const Entity& other) = default;

		template<typename T>
		T& AddComponent()
		{
			HZ_CORE_ASSERT(!HasComponent<T>(), "Entity already has component!");
			auto& component = _scene->_registry.emplace<T>(_entityHandle, *this);
			_scene->OnComponentAdded<T>(*this, component);
			return component;
		}

		template<typename T>
		void RemoveComponent()
		{
			HZ_CORE_ASSERT(HasComponent<T>(), "Entity does not have component!");
			_scene->_registry.remove<T>(_entityHandle);
		}

		template<typename T>
		T& GetComponent()
		{
			HZ_CORE_ASSERT(HasComponent<T>(), "Entity does not have component!");
			return _scene->_registry.get<T>(_entityHandle);
		}

		template<typename T>
		T* TryGetComponent()
		{
			return _scene->_registry.try_get<T>(_entityHandle);
		}

		template<typename T>
		bool HasComponent()
		{
			return _scene->_registry.has<T>(_entityHandle);
		}

		operator bool() const { return _entityHandle != entt::null; }
		operator entt::entity() const { return _entityHandle; }
		operator uint32_t() const { return (uint32_t)_entityHandle; }

		bool operator==(const Entity& other) const
		{
			return _entityHandle == other._entityHandle && _scene == other._scene;
		}

		bool operator!=(const Entity& other) const
		{
			return !(*this == other);
		}

		std::string& Name();
		int& Tag();
		int& Layer();

		TransformComponent& Transform();

	private:
		entt::entity _entityHandle{ entt::null };
		Scene* _scene = nullptr;
	};
}
