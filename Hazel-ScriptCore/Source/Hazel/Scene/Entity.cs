
using System;

namespace Hazel
{
	public class Entity
	{
		protected Entity()
		{
			Id = 0;
		}

		internal Entity(ulong id)
		{
			Id = id;
		}

		public readonly ulong Id;

		public Vector3 Position
		{
			get
			{
				InternalCalls.TransformComponent_GetPosition(Id, out Vector3 position);
				return position;
			}

			set => InternalCalls.TransformComponent_SetPosition(Id, ref value);
		}

		public bool HasComponent<T>() where T : Component, new()
		{
			Type componentType = typeof(T);
			return InternalCalls.Entity_HasComponent(Id, componentType);

		}

		public T GetComponent<T>() where T : Component, new()
		{
			if (!HasComponent<T>())
			{
				return null;
			}

			T component = new T() { Entity = this };
			return component;
		}
	}
}
