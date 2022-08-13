namespace Hazel
{
	public static class HMath
	{
		public static float Clamp(float value, float min, float max)
		{
			var clampedValue = value;
			clampedValue = clampedValue < min ? min : clampedValue;
			clampedValue = clampedValue > max ? max : clampedValue;
			return clampedValue;
		}
	}
}
