using Hazel;
using System;

namespace Sandbox
{
	public class FieldTester : Entity
	{
		public float flt = 69.0f;   // In Inpsector
		public double dbl = 4.269d; // In Inpsector
		public char chr = 'k';      // In Inpsector
		public bool bol = true;     // In Inpsector
		public sbyte sbyt = 127;    // In Inpsector
		public short sht = 2565;    // In Inpsector
		public int it = 1234;       // In Inpsector
		public long lng = 223456L;  // In Inpsector
		public byte byt = 255;      // In Inpsector
		public ushort usht = 25643; // In Inpsector
		public uint uit = 4321u;    // In Inpsector
		public ulong ulng = 3421ul; // In Inpsector
		public Vector2 vc2;
		public Vector3 vc3;
		public Vector4 vc4;
		public Color clr;
		public Entity ety;

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
		}
	}
}
