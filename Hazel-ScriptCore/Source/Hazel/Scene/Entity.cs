namespace Hazel
{
	public class Entity
	{
		public readonly ulong Id;

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
			Id = 0;
		}

		internal Entity(ulong id)
		{
			Id = id;
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
	}
}
