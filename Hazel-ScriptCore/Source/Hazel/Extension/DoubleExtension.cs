namespace Hazel
{
	public static class DoubleExtension
	{
		public static bool IsNearlyZero(this double value)
		{
			return System.Math.Abs(value) < double.Epsilon * double.Epsilon;
		}

		public static bool IsNearlyEqual(this double lhs, double rhs)
		{
			return System.Math.Abs(lhs - rhs) < double.Epsilon * double.Epsilon;
		}

		public static void Clamp(this ref double value, double min, double max)
		{
			value = value > max ? max : value;
			value = value < min ? min : value;
		}
	}
}
