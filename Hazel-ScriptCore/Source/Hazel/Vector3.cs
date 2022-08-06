
using System;

namespace Hazel
{
	public struct Vector3 : IEquatable<Vector3>
	{
		public float X;
		public float Y;
		public float Z;

		public static Vector3 Zero => new Vector3(0.0f);
		public static Vector3 Up => new Vector3(0.0f, 1.0f, 0.0f);
		public static Vector3 Right => new Vector3(1.0f, 0.0f, 0.0f);

		public Vector3(float x, float y, float z)
		{
			X = x;
			Y = y;
			Z = z;
		}

		public Vector3(float scalar)
		{
			X = scalar;
			Y = scalar;
			Z = scalar;
		}

		public static Vector3 operator *(Vector3 vector, float scalar)
		{
			return new Vector3(vector.X * scalar, vector.Y * scalar, vector.Z * scalar);
		}

		public static Vector3 operator +(Vector3 lhs, Vector3 rhs)
		{
			return new Vector3(lhs.X + rhs.X, lhs.Y + rhs.Y, lhs.Z + rhs.Z);
		}

		public override string ToString()
		{
			return $"({X:f},{Y:f},{Z:f})";
		}

		public override int GetHashCode()
		{
			return X.GetHashCode() ^ Y.GetHashCode() ^ Z.GetHashCode();
		}

		public override bool Equals(object other)
		{
			if (!(other is Vector3))
			{
				return false;
			}

			return Equals((Vector3)other);
		}

		public bool Equals(Vector3 other)
		{
			return X == other.X && Y == other.Y && Z == other.Z;
		}
	}
}
