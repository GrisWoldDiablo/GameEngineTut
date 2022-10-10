namespace Hazel
{
	public struct UUID : System.IEquatable<UUID>
	{
		private readonly ulong _uuid;

		public UUID(ulong uuid)
		{
			_uuid = uuid;
		}

		public bool Equals(UUID other)
		{
			return _uuid == other._uuid;
		}

		public override bool Equals(object other)
		{
			return other is UUID otherUUID && Equals(otherUUID);
		}

		public override int GetHashCode()
		{
			return _uuid.GetHashCode();
		}

		public override string ToString()
		{
			return _uuid.ToString();
		}

		public static implicit operator ulong(UUID value)
		{
			return value._uuid;
		}

		public static implicit operator UUID(ulong value)
		{
			return new UUID(value);
		}

		public static bool operator ==(UUID lhs, UUID rhs)
		{
			return lhs.Equals(rhs);
		}

		public static bool operator !=(UUID lhs, UUID rhs)
		{
			return !lhs.Equals(rhs);
		}
	}
}
