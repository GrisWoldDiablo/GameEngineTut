using Hazel;
using System;
using System.Reflection;

namespace Sandbox
{
	public class Player : Entity
	{
		public float MySpeed;

		private Rigidbody2DComponent _rigidbody;

		~Player()
		{
			Console.WriteLine($"{nameof(Player)} Finalizer(Destructor)");
		}

		private void OnCreate()
		{
			Console.WriteLine($"{GetType().FullName}.{MethodBase.GetCurrentMethod().Name} - {Id}");

			_rigidbody = GetComponent<Rigidbody2DComponent>();
			Console.WriteLine($"The entity Name: {Name}");
			Name = "C# name Player";
		}

		private void OnUpdate(float timestep)
		{
			if (!_rigidbody)
			{
				return;
			}

			float speed = 0.5f;
			var velocity = Vector3.Zero;

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

			velocity *= speed * timestep;

			_rigidbody.ApplyLinearImpulse(velocity);
		}
	}
}
