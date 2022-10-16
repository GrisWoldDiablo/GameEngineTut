using Hazel;
using System;

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
			Console.WriteLine($"New Entity : [{NewEntityName}]");
			var newEnityID = Create(NewEntityName ?? "Missing name");
			Console.WriteLine($"New Entity ID : [{newEnityID}]");

			var spriteComponent = newEnityID.AddComponent<SpriteRendererComponent>();
			spriteComponent.Color = NewEntityColor;

			_foundEntity = FindByName(EntityToFind ?? "");
			Console.WriteLine($"Find By Name : {_foundEntity}");
			if (_foundEntity is CirclePlayer player)
			{
				Console.WriteLine($"Find By Name  Player: {player}");
				EntityToTest = player;
				player.Color = NewEntityColor;
				_cr = player.GetComponent<CircleRendererComponent>();
			}
		}

		public void OnDestroy()
		{
			Console.WriteLine($"Destroy [{Name}<{Id}>]");
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
				Console.WriteLine($"DESTROY {_foundEntity}!");
				Destroy(_foundEntity);
			}
		}
	}
}
