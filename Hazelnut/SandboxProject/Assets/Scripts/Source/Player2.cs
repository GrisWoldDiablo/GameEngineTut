using Hazel;
using System;
using System.Reflection;

namespace Sandbox
{
	public class Player2 : Entity
	{
		readonly Vector3 _kVectorZ360 = new Vector3(0.0f, 0.0f, 360.0f);

		private void OnCreate()
		{
			Console.WriteLine($"{GetType().FullName}.{MethodBase.GetCurrentMethod().Name} - {Id}");
		}

		private void OnUpdate(float timestep)
		{
			float speed = 45.0f;
			var velocity = Vector3.Zero;

			// Rotation
			if (Input.IsKeyDown(KeyCode.LeftShift))
			{
				speed *= 5.0f;
			}

			if (Input.IsKeyDown(KeyCode.Q))
			{
				velocity.Z = 1.0f;
			}

			if (Input.IsKeyDown(KeyCode.E))
			{
				velocity.Z = -1.0f;
			}

			Transform.Rotation += velocity * speed * timestep;
			if (Transform.Rotation.Z < 0.0f)
			{
				Transform.Rotation += _kVectorZ360;
			}

			if (Transform.Rotation.Z > 360.0f)
			{
				Transform.Rotation -= _kVectorZ360;
			}
		}
	}
}
