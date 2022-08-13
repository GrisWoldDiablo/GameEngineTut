namespace Hazel
{
	public struct Vector2 : System.IEquatable<Vector2>
	{
		public float X;
		public float Y;

		public Vector2(float x, float y)
		{
			X = x;
			Y = y;
		}

		public Vector2(float scalar) : this(scalar, scalar) { }

		public Vector2(Vector3 vector3) : this(vector3.X, vector3.Y) { }

		public float Lenght() => Lenght(this);

		public void Normalize() => this = Normalize(this);

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

		public float this[int index]
		{
			get
			{
				switch (index)
				{
					case 0: return X;
					case 1: return Y;
					default:
						throw new System.IndexOutOfRangeException($"{nameof(Vector2)} invalid index!");
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
					default:
						throw new System.IndexOutOfRangeException($"{nameof(Vector2)} invalid index!");
				}
			}
		}

		#region statics
		public static Vector2 Zero => new Vector2(0.0f);
		public static Vector2 One => new Vector2(1.0f);
		public static Vector2 Up => new Vector2(0.0f, 1.0f);
		public static Vector2 Right => new Vector2(1.0f, 0.0f);

		public static Vector2 Lerp(Vector2 lhs, Vector2 rhs, float lerpAmount)
		{
			return (lhs * (1.0f - lerpAmount)) + (rhs * lerpAmount);
		}

		public static Vector2 Normalize(Vector2 value)
		{
			float lenght = Lenght(value);
			if (lenght > float.Epsilon)
			{
				return value / lenght;
			}

			return Zero;
		}

		public static float Lenght(Vector2 vector)
		{
			return (float)System.Math.Sqrt(Dot(vector, vector));
		}

		public static float Distance(Vector2 lhs, Vector2 rhs)
		{
			return Lenght(lhs - rhs);
		}

		public static float Dot(Vector2 lhs, Vector2 rhs)
		{
			return lhs.X * rhs.X + lhs.Y * rhs.Y;
		}

		public static Vector2 Reflect(Vector2 vector, Vector2 normal)
		{
			return vector - (normal * Dot(vector, normal) * 2.0f);
		}

		public static Vector2 Clamp(Vector2 value, Vector2 min, Vector2 max)
		{
			var x = value.X;
			x = x < min.X ? min.X : x;
			x = x > max.X ? max.X : x;

			var y = value.Y;
			y = y < min.Y ? min.Y : y;
			y = y > max.Y ? max.Y : y;

			return new Vector2(x, y);
		}

		public static Vector2 operator *(Vector2 vector, float scalar)
		{
			return new Vector2(vector.X * scalar, vector.Y * scalar);
		}

		public static Vector2 operator *(float scalar, Vector2 vector)
		{
			return new Vector2(vector.X * scalar, vector.Y * scalar);
		}

		public static Vector2 operator /(Vector2 value, float divisor)
		{
			return new Vector2(value.X / divisor, value.Y / divisor);
		}

		public static Vector2 operator +(Vector2 value, float addend)
		{
			return new Vector2(value.X + addend, value.Y + addend);
		}

		public static Vector2 operator -(Vector2 value, float minuend)
		{
			return new Vector2(value.X - minuend, value.Y - minuend);
		}

		public static Vector2 operator +(Vector2 lhs, Vector2 rhs)
		{
			return new Vector2(lhs.X + rhs.X, lhs.Y + rhs.Y);
		}

		public static Vector2 operator -(Vector2 lhs, Vector2 rhs)
		{
			return new Vector2(lhs.X - rhs.X, lhs.Y - rhs.Y);
		}

		public static Vector2 operator *(Vector2 lhs, Vector2 rhs)
		{
			return new Vector2(lhs.X * rhs.X, lhs.Y * rhs.Y);
		}

		public static Vector2 operator /(Vector2 lhs, Vector2 rhs)
		{
			return new Vector2(lhs.X / rhs.X, lhs.Y / rhs.Y);
		}

		public static Vector2 operator -(Vector2 value)
		{
			return Zero - value;
		}

		public static bool operator !=(Vector2 lhs, Vector2 rhs)
		{
			return (lhs.X != rhs.X ||
					lhs.Y != rhs.Y);
		}

		public static bool operator ==(Vector2 lhs, Vector2 rhs)
		{
			return (lhs.X == rhs.X &&
					lhs.Y == rhs.Y);
		}

		public static implicit operator Vector2(Vector3 vector3)
		{
			return new Vector2(vector3);
		}
		#endregion
	}
}
