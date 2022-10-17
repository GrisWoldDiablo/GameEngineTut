using System;

namespace Hazel
{
	// TODO Create HObject class to parent Component and Entity classes and move Id to it.
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
			return IsValid() ? $"{Name}<{Id}>" : "Null";
		}

		public override int GetHashCode()
		{
			return Id.GetHashCode();
		}

		public override bool Equals(object other)
		{
			return other is Entity otherEntity && CompareEntity(this, otherEntity);
		}

		public bool HasComponent(Type componentType)
		{
			if (!componentType.IsSubclassOf(typeof(Component)))
			{
				throw new ArgumentException($"Invalid type! {componentType} is not a subtype of {typeof(Component)}");
			}
			return InternalCalls.Entity_HasComponent(Id, componentType);
		}

		public bool HasComponent<T>() where T : Component, new()
		{
			return InternalCalls.Entity_HasComponent(Id, typeof(T));
		}

		public Component GetComponent(Type componentType)
		{
			if (!HasComponent(componentType))
			{
				return null;
			}

			var component = Activator.CreateInstance(componentType) as Component;
			var entityProperty = componentType.GetProperty(nameof(component.Entity));
			entityProperty.SetValue(component, this);
			return component;
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

		public bool Destroy()
		{
			return InternalCalls.Entity_Destroy(Id);
		}

		public bool IsValid()
		{
			return IsValid(this);
		}

		public static Entity Create(string name = "Entity")
		{
			InternalCalls.Entity_Create(ref name, out var newEntity);

			return newEntity;
		}

		public static bool Destroy(Entity entity)
		{
			return !entity || InternalCalls.Entity_Destroy(entity.Id);
		}

		/// <summary>
		/// This is slow as it will look through all entities and return the first one with matching name.
		/// </summary>
		public static Entity FindByName(string name)
		{
			if (InternalCalls.Entity_FindByName(ref name, out Entity entity))
			{
				return entity;
			}

			return null;
		}

		public static implicit operator bool(Entity entity)
		{
			return !CompareEntity(entity, null);
		}

		public static bool operator ==(Entity lhs, Entity rhs)
		{
			return CompareEntity(lhs, rhs);
		}

		public static bool operator !=(Entity lhs, Entity rhs)
		{
			return !CompareEntity(lhs, rhs);
		}

		private static bool CompareEntity(Entity lhs, Entity rhs)
		{
			var lhsIsNull = lhs is null;
			var rhsIsNull = rhs is null;

			if (lhsIsNull && rhsIsNull)
			{
				return true;
			}

			if (rhsIsNull)
			{
				return !IsValid(lhs);
			}

			if (lhsIsNull)
			{
				return !IsValid(rhs);
			}

			return lhs.Id == rhs.Id;
		}

		private static bool IsValid(Entity entity)
		{
			return InternalCalls.Entity_IsValid(entity.Id);
		}
	}
}
