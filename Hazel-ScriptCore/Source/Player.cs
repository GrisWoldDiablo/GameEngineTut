using Hazel;
using System;

namespace Sandbox
{
	public class Player : Entity
	{
		void OnCreate()
		{
			Console.WriteLine("Player.OnCreate");
		}

		void OnUpdate(float ts)
		{
			Console.WriteLine($"Player.OnUpdate: {ts}");
		}
	}
}
