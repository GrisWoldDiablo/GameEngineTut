
namespace Hazel
{
	public struct Vector3 : System.IEquatable<Vector3>
	{
		public float X;
		public float Y;
		public float Z;

		public Vector3(float x, float y, float z)
		{
			X = x;
			Y = y;
			Z = z;
		}

		public Vector3(float scalar) : this(scalar, scalar, scalar) { }

		public Vector3(Vector2 value, float z = 0.0f) : this(value.X, value.Y, z) { }

		public float Lenght() => Lenght(this);

		public void Normalize() => this = Normalize(this);

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

		public float this[int index]
		{
			get
			{
				switch (index)
				{
					case 0: return X;
					case 1: return Y;
					case 2: return Z;
					default:
						throw new System.IndexOutOfRangeException($"{nameof(Vector3)} invalid index!");
				}

			}
			set
			{
				switch (index)
				{
					case 0:
						X = value;
						break;
					case 1:
						Y = value;
						break;
					case 2:
						Z = value;
						break;
					default:
						throw new System.IndexOutOfRangeException($"{nameof(Vector3)} invalid index!");
				}
			}
		}

		#region statics
		public static Vector3 Zero => new Vector3(0.0f);
		public static Vector3 One => new Vector3(1.0f);
		public static Vector3 Up => new Vector3(0.0f, 1.0f, 0.0f);
		public static Vector3 Right => new Vector3(1.0f, 0.0f, 0.0f);

		public static Vector3 Lerp(Vector3 lhs, Vector3 rhs, float lerpAmount)
		{
			return (lhs * (1.0f - lerpAmount)) + (rhs * lerpAmount);
		}

		public static Vector3 Normalize(Vector3 value)
		{
			float lenght = Lenght(value);
			if (lenght > float.Epsilon)
			{
				return value / lenght;
			}

			return Zero;
		}

		public static float Lenght(Vector3 vector)
		{
			return (float)System.Math.Sqrt(Dot(vector, vector));
		}

		public static float Distance(Vector3 lhs, Vector3 rhs)
		{
			return Lenght(lhs - rhs);
		}

		public static float Dot(Vector3 lhs, Vector3 rhs)
		{
			return lhs.X * rhs.X + lhs.Y * rhs.Y + lhs.Z * rhs.Z;
		}

		public static Vector3 Cross(Vector3 lhs, Vector3 rhs)
		{
			return new Vector3(
				lhs.Y * rhs.Z - lhs.Z * rhs.Y,
				lhs.Z * rhs.X - lhs.X * rhs.Z,
				lhs.X * rhs.Y - lhs.Y * rhs.X);
		}

		public static Vector3 Reflect(Vector3 vector, Vector3 normal)
		{
			return vector - (normal * Dot(vector, normal) * 2.0f);
		}

		public static Vector3 Clamp(Vector3 value, Vector3 min, Vector3 max)
		{
			var x = value.X;
			x = x < min.X ? min.X : x;
			x = x > max.X ? max.X : x;

			var y = value.Y;
			y = y < min.Y ? min.Y : y;
			y = y > max.Y ? max.Y : y;

			var z = value.Z;
			z = z < min.Z ? min.Z : z;
			z = z > max.Z ? max.Z : z;

			return new Vector3(x, y, z);
		}

		public static Vector3 operator *(Vector3 value, float scalar)
		{
			return new Vector3(value.X * scalar, value.Y * scalar, value.Z * scalar);
		}

		public static Vector3 operator *(float scalar, Vector3 value)
		{
			return new Vector3(value.X * scalar, value.Y * scalar, value.Z * scalar);
		}

		public static Vector3 operator /(Vector3 value, float divisor)
		{
			return new Vector3(value.X / divisor, value.Y / divisor, value.Z / divisor);
		}

		public static Vector3 operator +(Vector3 value, float addend)
		{
			return new Vector3(value.X + addend, value.Y + addend, value.Z + addend);
		}

		public static Vector3 operator -(Vector3 value, float minuend)
		{
			return new Vector3(value.X - minuend, value.Y - minuend, value.Z - minuend);
		}

		public static Vector3 operator +(Vector3 lhs, Vector3 rhs)
		{
			return new Vector3(lhs.X + rhs.X, lhs.Y + rhs.Y, lhs.Z + rhs.Z);
		}

		public static Vector3 operator -(Vector3 lhs, Vector3 rhs)
		{
			return new Vector3(lhs.X - rhs.X, lhs.Y - rhs.Y, lhs.Z - rhs.Z);
		}

		public static Vector3 operator *(Vector3 lhs, Vector3 rhs)
		{
			return new Vector3(lhs.X * rhs.X, lhs.Y * rhs.Y, lhs.Z * rhs.Z);
		}

		public static Vector3 operator /(Vector3 lhs, Vector3 rhs)
		{
			return new Vector3(lhs.X / rhs.X, lhs.Y / rhs.Y, lhs.Z / rhs.Z);
		}

		public static Vector3 operator -(Vector3 value)
		{
			return Zero - value;
		}

		public static bool operator !=(Vector3 lhs, Vector3 rhs)
		{
			return (lhs.X != rhs.X ||
					lhs.Y != rhs.Y ||
					lhs.Z != rhs.Z);
		}

		public static bool operator ==(Vector3 lhs, Vector3 rhs)
		{
			return (lhs.X == rhs.X &&
					lhs.Y == rhs.Y &&
					lhs.Z == rhs.Z);
		}

		public static implicit operator Vector3(Vector2 value)
		{
			return new Vector3(value);
		}
		#endregion
	}
}
