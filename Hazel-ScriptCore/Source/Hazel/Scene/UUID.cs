using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Hazel
{
	public class UUID
	{
		private readonly ulong _uuid;

		public UUID(ulong uuid)
		{
			_uuid = uuid;
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
	}
}
