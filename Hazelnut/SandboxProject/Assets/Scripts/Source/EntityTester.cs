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

		public void OnCreate()
		{
			Console.WriteLine($"New Entity : {NewEntityName}");
			var newEnityID = Entity.CreateNew(NewEntityName);
			Console.WriteLine($"New Entity ID : {newEnityID}");

			var spriteComponent = newEnityID.AddComponent<SpriteRendererComponent>();
			spriteComponent.Color = NewEntityColor;

			var foundEntity = Entity.FindByName(EntityToFind);
			if (foundEntity != null)
			{
				var cr = foundEntity.GetComponent<CircleRendererComponent>();
				if (cr != null)
				{
					cr.Color = EntityToFindColor;
				}
			}
		}
	}
}
