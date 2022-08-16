using Hazel;

namespace Sandbox
{
	public class Camera : Entity
	{
		private void OnUpdate(float timestep)
		{
			float speed = 1.0f;
			var velocity = Vector3.Zero;

			if (Input.IsKeyDown(KeyCode.LeftShift))
			{
				speed = 5.0f;
			}

			if (Input.IsKeyDown(KeyCode.Up))
			{
				velocity.Y = 1.0f;
			}
			if (Input.IsKeyDown(KeyCode.Down))
			{
				velocity.Y = -1.0f;
			}
			if (Input.IsKeyDown(KeyCode.Left))
			{
				velocity.X = -1.0f;
			}
			if (Input.IsKeyDown(KeyCode.Right))
			{
				velocity.X = 1.0f;
			}

			var position = Transform.Position;
			velocity *= speed * timestep;
			position += velocity;
			Transform.Position = position;
		}
	}
}
