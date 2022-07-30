using Hazel;
using System;

namespace Sandbox
{
	public class Player : Entity
	{
		void OnCreate()
		{
			Console.WriteLine($"Player.OnCreate - {Id}");
		}

		void OnUpdate(float timestep)
		{
			Console.WriteLine($"Player.OnUpdate: {timestep}");


			float speed = 1.0f;
			Vector3 velocity = Vector3.Zero;

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

			Vector3 position = Position;
			position += velocity * speed * timestep;
			Position = position;
		}
	}
}
