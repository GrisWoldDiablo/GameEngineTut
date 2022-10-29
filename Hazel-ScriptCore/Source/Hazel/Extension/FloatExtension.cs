namespace Hazel
{
	public static class FloatExtention
	{
		public static bool IsNearlyZero(this float value)
		{
			return System.Math.Abs(value) <= float.Epsilon;
		}

		public static bool IsNearlyEqual(this float lhs, float rhs)
		{
			return System.Math.Abs(lhs - rhs) <= float.Epsilon;
		}

		public static void Clamp(this ref float value, float min, float max)
		{
			value = value > max ? max : value;
			value = value < min ? min : value;
		}
	}
}
