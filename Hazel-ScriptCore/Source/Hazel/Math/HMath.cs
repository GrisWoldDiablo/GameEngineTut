using System;
using System.Runtime.InteropServices.WindowsRuntime;

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

		public static float Lerp(float origin, float destination, float lerpValue)
		{
			return (origin * (1.0f - lerpValue)) + (destination * lerpValue);
		}

		public static float Clamp01(float value)
		{
			if (value < 0.0f)
			{
				return 0.0f;
			}

			if (value > 1.0f)
			{
				return 1.0f;
			}

			return value;
		}
	}
}