using Hazel;
using System;

namespace Sandbox
{
	public class FieldTester : Entity
	{
		public string @String = "Hello World";                                  // In Inpsector
		public float @float = 69.0f;                                            // In Inpsector
		public double @double = 4.269d;                                         // In Inpsector
		public char @char = 'k';                                                // In Inpsector
		public bool @bool = true;                                               // In Inpsector
		public sbyte @sbyte = 127;                                              // In Inpsector
		public short @short = 2565;                                             // In Inpsector
		public int @int = 1234;                                                 // In Inpsector
		public long @long = 223456L;                                            // In Inpsector
		public byte @byte = 255;                                                // In Inpsector
		public ushort @ushort = 25643;                                          // In Inpsector
		public uint @uint = 4321u;                                              // In Inpsector
		public ulong @ulong = 3421ul;                                           // In Inpsector
		public Vector2 @Vector2 = new Vector2(4.2f, 6.9f);                      // In Inpsector
		public Vector3 @Vector3 = new Vector3(4.2f, 6.9f, 420.69f);             // In Inpsector
		public Vector4 @Vector4 = new Vector4(4.2f, 6.9f, 420.69f, 69.420f);    // In Inpsector
		public Color @Color = new Color(0.15f, 0.89f, 0.91f, 0.69f);            // In Inpsector

		public Entity @Entity;

		static public int StaticInt;
		const int ConstInt = 2;
		readonly public int ReadonlyInt;

		public int PublicInt = 231; // In Inpsector
		private int PrivateInt;
		protected int ProtectedInt;
		internal int InternalInt;

		Vector3 vector;
		TransformComponent transformComponent;
		Entity entity;
		Color color;
		Player player;

		public int PublicIntProp { get; set; }
		private float PrivateFloatProp { get; set; }
		protected float ProtectedFloatProp { get; set; }
		internal float InternalFloatProp { get; set; }

		public int[] IntArray { get; }

		private readonly float _refreshRate = 2.0f;
		private float _timeleft = 0;
		private SpriteRendererComponent _spriteComponent;

		void OnCreate()
		{
			_spriteComponent = GetComponent<SpriteRendererComponent>();
		}

		void OnUpdate(float timeStep)
		{
			if (_timeleft <= 0.0f)
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
			_spriteComponent.Color = @Color;
		}
	}
}
