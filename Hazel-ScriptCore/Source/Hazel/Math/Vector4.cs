namespace Hazel
{
	public struct Vector4 : System.IEquatable<Vector4>
	{
		public float X;
		public float Y;
		public float Z;
		public float W;

		public Vector4(float x, float y, float z, float w)
		{
			X = x;
			Y = y;
			Z = z;
			W = w;
		}

		public Vector4(float scalar) : this(scalar, scalar, scalar, scalar) { }

		public Vector4(Vector2 vector2, float z = 0.0f, float w = 0.0f) : this(vector2.X, vector2.Y, z, w) { }

		public Vector4(Vector3 vector3, float w = 0.0f) : this(vector3.X, vector3.Y, vector3.Z, w) { }

		public Vector4(Vector4 vector4) : this(vector4.X, vector4.Y, vector4.Z, vector4.W) { }

		public float Lenght() => Lenght(this);

		public float LenghtSquared() => LenghtSquared(this);

		public void Normalize() => this = Normalize(this);

		public Vector4 Normalized() => Normalize(this);

		public override string ToString()
		{
			return $"({X:f}, {Y:f}, {Z:f}, {W:f})";
		}

		public override int GetHashCode()
		{
			return (X.GetHashCode() << 2) ^ (Y.GetHashCode() >> 1) ^ (Z.GetHashCode() << 2) ^ W.GetHashCode();
		}

		public override bool Equals(object other)
		{
			return other is Vector4 otherVector && Equals(otherVector);
		}

		public bool Equals(Vector4 other)
		{
			return X.IsNearlyEqual(other.X) && Y.IsNearlyEqual(other.Y) && Z.IsNearlyEqual(other.Z) && W.IsNearlyEqual(other.W);
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
				case 3: return W;
				default:
					throw new System.IndexOutOfRangeException($"{nameof(Vector4)} invalid index!");
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
				case 3:
					W = value;
					break;
				default:
					throw new System.IndexOutOfRangeException($"{nameof(Vector4)} invalid index!");
				}
			}
		}

		#region statics
		public static Vector4 Zero => new Vector4(0.0f);
		public static Vector4 One => new Vector4(1.0f);

		public static Vector4 Lerp(Vector4 lhs, Vector4 rhs, float lerpAmount)
		{
			return (lhs * (1.0f - lerpAmount)) + (rhs * lerpAmount);
		}

		public static Vector4 Normalize(Vector4 value)
		{
			float lenght = Lenght(value);
			if (!lenght.IsNearlyZero())
			{
				return value / lenght;
			}

			return Zero;
		}

		public static float Lenght(Vector4 vector)
		{
			return (float)System.Math.Sqrt(Dot(vector, vector));
		}

		public static float LenghtSquared(Vector4 vector)
		{
			return Dot(vector, vector);
		}

		public static Vector4 ClampLenght(Vector4 vector, float maxLenght)
		{
			if (LenghtSquared(vector) < maxLenght * maxLenght)
			{
				return Normalize(vector) * maxLenght;
			}

			return vector;
		}

		public static float Distance(Vector4 lhs, Vector4 rhs)
		{
			return Lenght(lhs - rhs);
		}

		public static float Dot(Vector4 lhs, Vector4 rhs)
		{
			return lhs.X * rhs.X + lhs.Y * rhs.Y + lhs.Z * rhs.Z + lhs.W * rhs.W;
		}

		public static Vector4 Reflect(Vector4 vector, Vector4 normal)
		{
			return vector - (normal * Dot(vector, normal) * 2.0f);
		}

		public static Vector4 Clamp(Vector4 value, Vector4 min, Vector4 max)
		{
			value.X.Clamp(min.X, max.X);
			value.Y.Clamp(min.Y, max.Y);
			value.Z.Clamp(min.Z, max.Z);
			value.W.Clamp(min.W, max.W);
			return new Vector4(value);
		}

		public static Vector4 operator *(Vector4 value, float scalar)
		{
			return new Vector4(value.X * scalar, value.Y * scalar, value.Z * scalar, value.W * scalar);
		}

		public static Vector4 operator *(float scalar, Vector4 value)
		{
			return new Vector4(value.X * scalar, value.Y * scalar, value.Z * scalar, value.W * scalar);
		}

		public static Vector4 operator /(Vector4 value, float divisor)
		{
			if (divisor.IsNearlyZero())
			{
				return Zero;
			}
			return new Vector4(value.X / divisor, value.Y / divisor, value.Z / divisor, value.W / divisor);
		}

		public static Vector4 operator +(Vector4 value, float addend)
		{
			return new Vector4(value.X + addend, value.Y + addend, value.Z + addend, value.W + addend);
		}

		public static Vector4 operator -(Vector4 value, float minuend)
		{
			return new Vector4(value.X - minuend, value.Y - minuend, value.Z - minuend, value.W - minuend);
		}

		public static Vector4 operator +(Vector4 lhs, Vector4 rhs)
		{
			return new Vector4(lhs.X + rhs.X, lhs.Y + rhs.Y, lhs.Z + rhs.Z, lhs.W + rhs.W);
		}

		public static Vector4 operator -(Vector4 lhs, Vector4 rhs)
		{
			return new Vector4(lhs.X - rhs.X, lhs.Y - rhs.Y, lhs.Z - rhs.Z, lhs.W - rhs.W);
		}

		public static Vector4 operator *(Vector4 lhs, Vector4 rhs)
		{
			return new Vector4(lhs.X * rhs.X, lhs.Y * rhs.Y, lhs.Z * rhs.Z, lhs.W * rhs.W);
		}

		public static Vector4 operator /(Vector4 lhs, Vector4 rhs)
		{
			var x = rhs.X.IsNearlyZero() ? 0.0f : lhs.X / rhs.X;
			var y = rhs.Y.IsNearlyZero() ? 0.0f : lhs.Y / rhs.Y;
			var z = rhs.Z.IsNearlyZero() ? 0.0f : lhs.Z / rhs.Z;
			var w = rhs.W.IsNearlyZero() ? 0.0f : lhs.W / rhs.W;
			return new Vector4(x, y, z, w);
		}

		public static Vector4 operator -(Vector4 value)
		{
			return Zero - value;
		}

		public static bool operator ==(Vector4 lhs, Vector4 rhs)
		{
			return lhs.Equals(rhs);
		}

		public static bool operator !=(Vector4 lhs, Vector4 rhs)
		{
			return !lhs.Equals(rhs);
		}

		public static implicit operator Vector4(Vector2 value)
		{
			return new Vector4(value);
		}

		public static implicit operator Vector4(Vector3 value)
		{
			return new Vector4(value);
		}
		#endregion
	}
}