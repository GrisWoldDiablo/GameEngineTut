using System;

namespace Hazel
{
	public struct Vector2 : IEquatable<Vector2>
	{
		public float X;
		public float Y;

		public static Vector2 Zero => new Vector2(0.0f);
		public static Vector2 Up => new Vector2(0.0f, 1.0f);
		public static Vector2 Right => new Vector2(1.0f, 0.0f);

		public Vector2(float x, float y)
		{
			X = x;
			Y = y;
		}

		public Vector2(float scalar)
		{
			X = scalar;
			Y = scalar;
		}

		public static Vector2 operator *(Vector2 vector, float scalar)
		{
			return new Vector2(vector.X * scalar, vector.Y * scalar);
		}

		public static Vector2 operator +(Vector2 lhs, Vector2 rhs)
		{
			return new Vector2(lhs.X + rhs.X, lhs.Y + rhs.Y);
		}

		public static implicit operator Vector2(Vector3 vector3)
		{
			return new Vector2(vector3.X, vector3.Y);
		}

		public override string ToString()
		{
			return $"({X:f},{Y:f})";
		}

		public override int GetHashCode()
		{
			return X.GetHashCode() ^ Y.GetHashCode();
		}

		public override bool Equals(object other)
		{
			if (!(other is Vector2))
			{
				return false;
			}

			return Equals((Vector2)other);
		}

		public bool Equals(Vector2 other)
		{
			return X == other.X && Y == other.Y;
		}
	}
}
