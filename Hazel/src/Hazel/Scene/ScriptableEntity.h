#pragma once
#include "Entity.h"

namespace Hazel
{
	class ScriptableEntity
	{
	public:
		virtual ~ScriptableEntity() {}

		template<typename T>
		T& GetComponent()
		{
			return _entity.GetComponent<T>();
		}

		virtual void OnCreate() {}
		virtual void OnDestroy() {}
		virtual void OnUpdate(Timestep timestep) {}
		virtual std::string GetClassFilePath() = 0;

	private:
		Entity _entity;
		friend class Scene;
	};
}
