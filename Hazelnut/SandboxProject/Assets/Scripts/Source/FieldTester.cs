using Hazel;

namespace Sandbox
{
	public class FieldTester : Entity
	{
		float flt;
		double dbl;
		char chr;
		bool bol;
		sbyte sbyt;
		short sht;
		int it;
		long lng;
		byte byt;
		ushort usht;
		uint uit;
		ulong ulng;
		Vector2 vc2;
		Vector3 vc3;
		Vector4 vc4;
		Color clr;
		Entity ety;

		static int StaticInt;
		const int ConstInt = 2;
		readonly int ReadonlyInt;
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
	}
}
