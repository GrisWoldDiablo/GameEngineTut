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
			Logger.Debug($"New Entity : [{NewEntityName}]");
			var newEnityID = Create(NewEntityName ?? "Missing name");
			Logger.Debug($"New Entity ID : [{newEnityID}]");

			var spriteComponent = newEnityID.AddComponent<SpriteRendererComponent>();
			spriteComponent.Color = NewEntityColor;

			_foundEntity = FindByName(EntityToFind ?? "");
			Logger.Debug($"Find By Name : {_foundEntity}");
			if (_foundEntity)
			{
				var player = _foundEntity.As<CirclePlayer>();
				Logger.Debug($"Find By Name  Player: {player}");
				EntityToTest = player;
				player.Color = NewEntityColor;
				_cr = player.GetComponent<CircleRendererComponent>();
			}
		}

		public void OnDestroy()
		{
			Logger.Debug($"Destroy [{Name}<{Id}>]");
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
				Logger.Debug($"DESTROY {_foundEntity}!");
				Destroy(_foundEntity);
			}
		}
	}
}
