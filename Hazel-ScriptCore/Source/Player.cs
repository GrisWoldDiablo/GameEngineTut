using Hazel;
using System;
using System.Reflection;

namespace Sandbox
{
	public class Player : Entity
	{
		private TransformComponent _transform;

		void OnCreate()
		{
			Console.WriteLine($"{GetType().FullName}.{MethodBase.GetCurrentMethod().Name} - {Id}");

			_transform = GetComponent<TransformComponent>();
			_transform.Position = new Vector3(0.0f);
		}

		void OnUpdate(float timestep)
		{
			float speed = 1.0f;
			Vector3 velocity = Vector3.Zero;
			if (Input.IsKeyDown(KeyCode.LeftShift))
			{
				speed = 5.0f;
			}

			if (Input.IsKeyDown(KeyCode.W))
			{
				velocity.Y = 1.0f;
			}
			if (Input.IsKeyDown(KeyCode.S))
			{
				velocity.Y = -1.0f;
			}
			if (Input.IsKeyDown(KeyCode.A))
			{
				velocity.X = -1.0f;
			}
			if (Input.IsKeyDown(KeyCode.D))
			{
				velocity.X = 1.0f;
			}
			if (Input.IsKeyDown(KeyCode.SPACE))
			{
				Console.WriteLine("Jump!");
			}

			Vector3 position = Position;
			position += velocity * speed * timestep;
			Position = position;
		}
	}
}
