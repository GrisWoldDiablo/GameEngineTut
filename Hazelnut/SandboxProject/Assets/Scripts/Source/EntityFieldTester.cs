﻿using Hazel;
using System;

namespace Sandbox
{
	public class EntityChild : Entity { }
	public class EntityGrandChild : EntityChild { }

	public class EntityFieldTester : Entity
	{
		public Entity Entity;
		public Camera Camera;
		public CirclePlayer CirclePlayer;
		public FieldTester FieldTester;
		public Player Player;
		public Player2 Player2;
		public EntityChild EntityChild;
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
				Console.WriteLine("--------------");
				foreach (var field in GetType().GetFields())
				{
					Console.WriteLine($"{field.Name} : {field.GetValue(this)}");
				}
				Console.WriteLine("--------------");

				_timeleft = _refreshRate;
			}

			_timeleft -= timeStep;
		}
	}
}
