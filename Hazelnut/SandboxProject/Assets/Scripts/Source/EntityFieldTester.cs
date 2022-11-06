using Hazel;

namespace Sandbox
{
	public class EntityChild : Entity { }
	public class EntitySmallChild : EntityChild { }
	public class EntityGrandChild : EntitySmallChild { }

	public class EntityFieldTester : Entity
	{
		public Entity Entity;
		public Camera Camera;
		public CirclePlayer CirclePlayer;
		public FieldTester FieldTester;
		public Player Player;
		public Player2 Player2;
		public EntityChild EntityChild;
		public EntitySmallChild EntitySmallChild;
		public EntityGrandChild EntityGrandChild;
		public AudioListener AudioListener;
		public AudioTester AudioTester;

		public bool SHOULD_LOG_FIELDS;
		private float _timeleft;
		private float _refreshRate = 5.0f;

		void OnUpdate(float timeStep)
		{
			if (SHOULD_LOG_FIELDS && _timeleft <= 0.0f)
			{
				Debug.Log("--------------");
				foreach (var field in GetType().GetFields())
				{
					Debug.Log($"{field.Name} : {field.GetValue(this)}");
				}
				Debug.Log("--------------");

				_timeleft = _refreshRate;
			}

			_timeleft -= timeStep;
		}
	}
}
