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
}
