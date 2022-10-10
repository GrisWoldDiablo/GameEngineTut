using Hazel;
using System;

namespace Sandbox
{
	public class EntityTester : Entity
	{
		public string NewEntityName;
		public Color NewEntityColor;
		public string EntityToFind;
		public Color EntityToFindColor;

		public float lifeTime = 5.0f;
		private Entity _foundEntity;

		public void OnCreate()
		{
			Console.WriteLine($"New Entity : [{NewEntityName}]");
			var newEnityID = Create(NewEntityName);
			Console.WriteLine($"New Entity ID : [{newEnityID}]");

			var spriteComponent = newEnityID.AddComponent<SpriteRendererComponent>();
			spriteComponent.Color = NewEntityColor;

			_foundEntity = FindByName(EntityToFind);
			if (_foundEntity != null)
			{
				var cr = _foundEntity.GetComponent<CircleRendererComponent>();
				if (cr != null)
				{
					cr.Color = EntityToFindColor;
				}
			}
		}

		public void OnDestroy()
		{
			Console.WriteLine($"Destroy [{Name}<{Id}>]");
		}

		public void OnUpdate(float timestep)
		{
			if (!_foundEntity)
			{
				return;
			}

			lifeTime -= timestep;

			if (lifeTime < 0.0f)
			{
				Destroy(_foundEntity.Id);
			}
		}
	}
}
