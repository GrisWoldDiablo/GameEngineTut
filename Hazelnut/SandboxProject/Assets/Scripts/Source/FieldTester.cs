using Hazel;
using System;

namespace Sandbox
{
	public class FieldTester : Entity
	{
		public float flt = 69.0f;
		public double dbl;
		public char chr = 'k';
		public bool bol;
		public sbyte sbyt;
		public short sht;
		public int it;
		public long lng;
		public byte byt;
		public ushort usht;
		public uint uit;
		public ulong ulng;
		public Vector2 vc2;
		public Vector3 vc3;
		public Vector4 vc4;
		public Color clr;
		public Entity ety;

		static public int StaticInt;
		const int ConstInt = 2;
		readonly public int ReadonlyInt;
		public int PublicInt;
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
				Console.WriteLine($"CHAR {chr}");

				_timeleft = _refreshRate;
			}

			_timeleft -= timeStep;
		}
	}
}
