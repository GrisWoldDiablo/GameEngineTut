namespace Hazel
{
	public class Entity
	{
		public readonly UUID Id;

		public string Name
		{
			get
			{
				InternalCalls.Entity_GetName(Id, out var name);
				return name;
			}

			set => InternalCalls.Entity_SetName(Id, ref value);
		}

		private TransformComponent _transform;

		public TransformComponent Transform
		{
			get
			{
				if (_transform == null)
				{
					_transform = GetComponent<TransformComponent>();
				}

				return _transform;
			}

			private set => _transform = value;
		}

		protected Entity()
		{
			Id = new UUID(0);
		}

		internal Entity(ulong id)
		{
			Id = new UUID(id);
		}

		public override string ToString()
		{
			return $"{Name}<{Id}>";
		}

		public bool HasComponent<T>() where T : Component, new()
		{
			return InternalCalls.Entity_HasComponent(Id, typeof(T));
		}

		public T GetComponent<T>() where T : Component, new()
		{
			if (!HasComponent<T>())
			{
				return null;
			}

			return new T() { Entity = this };
		}

		public T AddComponent<T>() where T : Component, new()
		{
			if (HasComponent<T>())
			{
				return null;
			}

			InternalCalls.Entity_AddComponent(Id, typeof(T));

			return new T() { Entity = this };
		}

		public static Entity CreateNew(string name = "Entity")
		{
			InternalCalls.Entity_CreateNew(ref name, out var newId);

			return new Entity(newId);
		}

		/// <summary>
		/// This is slow as it will look through all entities and return the first one with matching name.
		/// </summary>
		public static Entity FindByName(string name)
		{
			InternalCalls.Entity_FindByName(ref name, out var newId);

			return newId != 0 ? new Entity(newId) : null;
		}
	}
}
