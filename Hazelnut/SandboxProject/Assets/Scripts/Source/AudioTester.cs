using Hazel;
using System;

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
		private bool _isWDown;
		private bool _isDDown;
		private bool _isADown;

		void OnCreate()
		{
			if (LeftSource)
			{
				_leftSourceAudio = LeftSource.GetComponent<AudioSourceComponent>();
				Console.WriteLine($"Left : {_leftSourceAudio?.Path}");
			}

			if (RightSource)
			{
				_rightSourceAudio = RightSource.GetComponent<AudioSourceComponent>();
				Console.WriteLine($"Right : {_rightSourceAudio?.Path}");
			}

			if (MusicSource)
			{
				_musicSourceAudio = MusicSource.GetComponent(typeof(AudioSourceComponent)) as AudioSourceComponent;
				Console.WriteLine($"Music : {_musicSourceAudio?.Path}");
				Path = _musicSourceAudio?.Path;
				Console.WriteLine($" Has Type {typeof(TransformComponent)}: {MusicSource.HasComponent(typeof(TransformComponent))}");
				Console.WriteLine($" Has Type {typeof(CircleRendererComponent)}: {MusicSource.HasComponent(typeof(CircleRendererComponent))}");
			}
		}

		void OnUpdate(float timestep)
		{
			if (Input.IsKeyDown(KeyCode.A))
			{
				if (!_isADown && _leftSourceAudio)
				{
					_isADown = true;
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
			else
			{
				_isADown = false;
			}

			if (Input.IsKeyDown(KeyCode.D))
			{
				if (!_isDDown && _rightSourceAudio)
				{
					_isDDown = true;
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
			else
			{
				_isDDown = false;
			}

			if (Input.IsKeyDown(KeyCode.W))
			{
				if (!_isWDown && _musicSourceAudio)
				{
					_isWDown = true;
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
			else
			{
				_isWDown = false;
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
