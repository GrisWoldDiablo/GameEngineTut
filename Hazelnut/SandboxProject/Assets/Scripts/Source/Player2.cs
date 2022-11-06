using Hazel;
using System.Reflection;

namespace Sandbox
{
	public class Player2 : Entity
	{
		readonly Vector3 _kVectorZ360 = new Vector3(0.0f, 0.0f, 360.0f);
		private SpriteRendererComponent _spriteRenderer;

		private void OnCreate()
		{
			Debug.Log($"{GetType().FullName}.{MethodBase.GetCurrentMethod().Name} - {Id}");
			_spriteRenderer = GetComponent<SpriteRendererComponent>();
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


			// Sprite Renderer
			if (!_spriteRenderer)
			{
				return;
			}

			if (Input.IsKeyDown(KeyCode.SPACE))
			{
				_spriteRenderer.Color = Color.Random();
			}

			speed = 5.0f;

			if (!Input.IsKeyDown(KeyCode.LeftShift))
			{
				var tiling = _spriteRenderer.Tiling;

				if (Input.IsKeyDown(KeyCode.A))
				{
					tiling.X += -speed * timestep;
				}

				if (Input.IsKeyDown(KeyCode.D))
				{
					tiling.X += speed * timestep;
				}

				if (Input.IsKeyDown(KeyCode.W))
				{
					tiling.Y += speed * timestep;
				}

				if (Input.IsKeyDown(KeyCode.S))
				{
					tiling.Y += -speed * timestep;
				}

				_spriteRenderer.Tiling = tiling;
			}

			if (Input.IsKeyDown(KeyCode.LeftShift))
			{
				var scale = Transform.Scale;

				if (Input.IsKeyDown(KeyCode.A))
				{
					scale.X += -speed * timestep;
				}

				if (Input.IsKeyDown(KeyCode.D))
				{
					scale.X += speed * timestep;
				}

				if (Input.IsKeyDown(KeyCode.W))
				{
					scale.Y += speed * timestep;

				}

				if (Input.IsKeyDown(KeyCode.S))
				{
					scale.Y += -speed * timestep;
				}

				Transform.Scale = scale;
			}
		}
	}
}
