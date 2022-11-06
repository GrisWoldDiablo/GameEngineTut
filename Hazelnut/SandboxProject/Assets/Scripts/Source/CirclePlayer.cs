using Hazel;
using System.Reflection;

namespace Sandbox
{
	public class CirclePlayer : Entity
	{
		public Color Color;

		private CircleRendererComponent _circleRenderer;

		private void OnCreate()
		{
			Debug.Log($"{GetType().FullName}.{MethodBase.GetCurrentMethod().Name} - {Id}");

			_circleRenderer = GetComponent<CircleRendererComponent>();
		}

		public void OnDestroy()
		{
			Debug.Log($"Destroy [{Name}<{Id}>]");
		}

		private void OnUpdate(float timestep)
		{
			if (!_circleRenderer)
			{
				return;
			}

			// Sprite Renderer
			if (Input.IsKeyDown(KeyCode.SPACE))
			{
				_circleRenderer.Color = Color.Random();
			}

			float speed = 5.0f;

			if (!Input.IsKeyDown(KeyCode.LeftShift))
			{
				var thickness = _circleRenderer.Thickness;
				var fade = _circleRenderer.Fade;

				if (Input.IsKeyDown(KeyCode.A))
				{
					thickness += -speed * timestep;
				}

				if (Input.IsKeyDown(KeyCode.D))
				{
					thickness += speed * timestep;
				}

				if (Input.IsKeyDown(KeyCode.W))
				{
					fade += speed * timestep;
				}

				if (Input.IsKeyDown(KeyCode.S))
				{
					fade += -speed * timestep;
				}

				_circleRenderer.Thickness = thickness;
				_circleRenderer.Fade = fade;
			}
		}
	}
}
