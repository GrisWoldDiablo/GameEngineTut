using Hazel;

namespace Sandbox
{
	public class AudioTester : Entity
	{
		public Entity LeftSource;
		public Entity RightSource;
		public Entity MusicSource;

		public float Gain;
		public float Pitch;
		public bool IsLoop;
		public bool Is3D;
		public float Offset;
		public string Path;
		public float Lenght;
		public int State;

		private AudioSourceComponent _leftSourceAudio;
		private AudioSourceComponent _rightSourceAudio;
		private AudioSourceComponent _musicSourceAudio;

		void OnCreate()
		{
			if (LeftSource)
			{
				_leftSourceAudio = LeftSource.GetComponent<AudioSourceComponent>();
				Logger.Debug($"Left : {(_leftSourceAudio ? _leftSourceAudio.Path : null)}");
			}

			if (RightSource)
			{
				_rightSourceAudio = RightSource.GetComponent<AudioSourceComponent>();
				Logger.Debug($"Right : {(_rightSourceAudio ? _rightSourceAudio.Path : null)}");
			}

			if (MusicSource)
			{
				_musicSourceAudio = MusicSource.GetComponent(typeof(AudioSourceComponent)) as AudioSourceComponent;

				Logger.Debug($"Music : {(_musicSourceAudio ? _musicSourceAudio.Path : null)}");
				Path = _musicSourceAudio ? _musicSourceAudio.Path : null;
				Logger.Debug($" Has Type {typeof(TransformComponent)}: {MusicSource.HasComponent(typeof(TransformComponent))}");
				Logger.Debug($" Has Type {typeof(CircleRendererComponent)}: {MusicSource.HasComponent(typeof(CircleRendererComponent))}");
			}
		}

		void OnUpdate(float timestep)
		{
			if (Input.IsKeyPressed(KeyCode.A))
			{
				if (_leftSourceAudio)
				{
					switch (_leftSourceAudio.State)
					{
						case AudioSourceState.None:
						case AudioSourceState.Initial:
						case AudioSourceState.Stopped:
						case AudioSourceState.Pause:
							_leftSourceAudio.Play();
							break;
						case AudioSourceState.Playing:
							_leftSourceAudio.Stop();
							break;
					}

					if (Input.IsKeyDown(KeyCode.LeftShift))
					{
						_leftSourceAudio.IsLoop = !_leftSourceAudio.IsLoop;
					}
				}
			}

			if (Input.IsKeyPressed(KeyCode.D))
			{
				if (_rightSourceAudio)
				{
					switch (_rightSourceAudio.State)
					{
						case AudioSourceState.None:
						case AudioSourceState.Initial:
						case AudioSourceState.Stopped:
						case AudioSourceState.Pause:
							_rightSourceAudio.Play();
							break;
						case AudioSourceState.Playing:
							_rightSourceAudio.Stop();
							break;
					}

					if (Input.IsKeyDown(KeyCode.LeftShift))
					{
						_rightSourceAudio.Is3D = !_rightSourceAudio.Is3D;
					}
				}
			}

			if (Input.IsKeyPressed(KeyCode.W))
			{
				if (_musicSourceAudio)
				{
					switch (_musicSourceAudio.State)
					{
						case AudioSourceState.None:
						case AudioSourceState.Initial:
						case AudioSourceState.Stopped:
						case AudioSourceState.Pause:
							_musicSourceAudio.Play();
							break;
						case AudioSourceState.Playing:
							_musicSourceAudio.Stop();
							break;
					}
				}
			}

			if (_musicSourceAudio)
			{
				Gain = _musicSourceAudio.Gain;
				Pitch = _musicSourceAudio.Pitch;
				IsLoop = _musicSourceAudio.IsLoop;
				Is3D = _musicSourceAudio.Is3D;
				Offset = _musicSourceAudio.Offset;
				Lenght = _musicSourceAudio.Length;
				State = (int)_musicSourceAudio.State;
			}
		}
	}
}
