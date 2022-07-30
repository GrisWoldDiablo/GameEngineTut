namespace Hazel
{
	public struct Vector3
	{
		public float X;
		public float Y;
		public float Z;

		public static Vector3 Zero => new Vector3(0.0f);

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

		public override string ToString()
		{
			return $"({X:F2},{Y:F2},{Z:F2})";
		}

		public static Vector3 operator *(Vector3 vector, float scalar)
		{
			return new Vector3(vector.X * scalar, vector.Y * scalar, vector.Z * scalar);
		}

		public static Vector3 operator +(Vector3 lhs, Vector3 rhs)
		{
			return new Vector3(lhs.X + rhs.X, lhs.Y + rhs.Y, lhs.Z + rhs.Z);
		}
	}

	public class Entity
	{
		protected Entity()
		{
			Id = 0;
		}

		internal Entity(ulong id)
		{
			Id = id;
		}

		public readonly ulong Id;

		public Vector3 Position
		{
			get
			{
				InternalCalls.Entity_GetPosition(Id, out Vector3 position);
				return position;
			}

			set => InternalCalls.Entity_SetPosition(Id, ref value);
		}
	}
}
