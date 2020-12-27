#pragma once
#include "Entity.h"

namespace Hazel
{
	class ScriptableEntity
	{
	public:

		template<typename T>
		T& GetComponent()
		{
			return _entity.GetComponent<T>();
		}

	private:
		Entity _entity;
		friend class Scene;
	};
}
