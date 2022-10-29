using System;

namespace Hazel
{
	public static class HMath
	{
		public static T Clamp<T>(T value, T min, T max) where T : IComparable
		{
			var clampedValue = value;
			clampedValue = clampedValue.CompareTo(min) < 0 ? min : clampedValue;
			clampedValue = clampedValue.CompareTo(max) > 0 ? max : clampedValue;
			return clampedValue;
		}

		public static bool IsNearlyEqual(float lhs, float rhs)
		{
			return Math.Abs(lhs - rhs) < float.Epsilon * float.Epsilon;
		}
		
		public static bool IsNearlyEqual(double lhs, double rhs)
		{
			return Math.Abs(lhs - rhs) < double.Epsilon * double.Epsilon;
		}
	}
}
