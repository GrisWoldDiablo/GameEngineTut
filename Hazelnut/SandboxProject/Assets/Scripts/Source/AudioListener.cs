using Hazel;

namespace Sandbox
{
	public class AudioListener : Entity
	{
		private AudioListenerComponent _audioListener;

		private void OnCreate()
		{
			_audioListener = GetComponent<AudioListenerComponent>();
		}

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

			if (Input.IsKeyDown(KeyCode.SPACE))
			{
				_audioListener.IsVisibleInGame = !_audioListener.IsVisibleInGame;
			}
		}
	}
}
