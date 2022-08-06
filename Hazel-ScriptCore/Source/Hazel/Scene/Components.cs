namespace Hazel
{
	public class Component
	{
		public Entity Entity { get; internal set; }
	}

	public class TransformComponent : Component
	{
		public Vector3 Position
		{
			get
			{
				InternalCalls.TransformComponent_GetPosition(Entity.Id, out var position);
				return position;
			}

			set => InternalCalls.TransformComponent_SetPosition(Entity.Id, ref value);
		}
	}

	public class Rigidbody2DComponent : Component
	{
		public void ApplyLinearImpulse(Vector2 impulse, Vector2 worldPoint, bool wake = true)
		{
			InternalCalls.Rigidbody2DComponent_ApplyLinearImpulse(Entity.Id, ref impulse, ref worldPoint, wake);
		}

		public void ApplyLinearImpulse(Vector2 impulse, bool wake = true)
		{
			InternalCalls.Rigidbody2DComponent_ApplyLinearImpulseToCenter(Entity.Id, ref impulse, wake);
		}
	}
}
