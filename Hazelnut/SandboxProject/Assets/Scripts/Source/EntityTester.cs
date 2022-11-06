using Hazel;

namespace Sandbox
{
	public class EntityTester : Entity
	{
		public string NewEntityName = string.Empty;
		public Color NewEntityColor;
		public string EntityToFind = string.Empty;
		public Color EntityToFindColor;
		public Entity EntityToTest;

		public float lifeTime = 5.0f;
		private Entity _foundEntity;
		private CircleRendererComponent _cr;

		public void OnCreate()
		{
			Debug.Log($"New Entity : [{NewEntityName}]");
			var newEnityID = Create(NewEntityName ?? "Missing name");
			Debug.Log($"New Entity ID : [{newEnityID}]");

			var spriteComponent = newEnityID.AddComponent<SpriteRendererComponent>();
			spriteComponent.Color = NewEntityColor;

			_foundEntity = FindByName(EntityToFind ?? "");
			Debug.Log($"Find By Name : {_foundEntity}");
			if (_foundEntity)
			{
				var player = _foundEntity.As<CirclePlayer>();
				Debug.Log($"Find By Name  Player: {player}");
				EntityToTest = player;
				player.Color = NewEntityColor;
				_cr = player.GetComponent<CircleRendererComponent>();
			}
		}

		public void OnDestroy()
		{
			Debug.Log($"Destroy [{Name}<{Id}>]");
		}

		public void OnUpdate(float timestep)
		{
			if (EntityToTest)
			{
				var position = EntityToTest.Transform.Position;
				position.X += 0.5f * timestep;
				EntityToTest.Transform.Position = position;
			}

			if (lifeTime < 2.5f && _cr)
			{
				_cr.Color = EntityToFindColor;
			}

			lifeTime -= timestep;

			if (!_foundEntity)
			{
				return;
			}

			if (lifeTime < 0.0f)
			{
				Debug.Log($"DESTROY {_foundEntity}!");
				Destroy(_foundEntity);
			}
		}
	}
}
