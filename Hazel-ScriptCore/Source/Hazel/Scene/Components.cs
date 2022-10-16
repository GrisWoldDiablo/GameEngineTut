namespace Hazel
{
	// TODO Create HObject class to parent Component and Entity classes
	public class Component
	{
		public Entity Entity { get; internal set; }

		public override int GetHashCode()
		{
			return (Entity.GetHashCode() << 2) ^ (Entity.GetHashCode() >> 1);
		}

		public override bool Equals(object obj)
		{
			return obj is Component component && Entity.Equals(component.Entity);
		}

		public static implicit operator bool(Component component)
		{
			return !(component is null) && component.Entity;
		}

		public static bool operator ==(Component lhs, Component rhs)
		{
			return CompareComponent(lhs, rhs);
		}

		public static bool operator !=(Component lhs, Component rhs)
		{
			return !CompareComponent(lhs, rhs);
		}

		private static bool CompareComponent(Component lhs, Component rhs)
		{
			var lhsIsNull = lhs is null;
			var rhsIsNull = rhs is null;

			if (lhsIsNull && rhsIsNull)
			{
				return true;
			}

			if (rhsIsNull && !lhsIsNull)
			{
				return !lhs.Entity;
			}

			if (lhsIsNull && !rhsIsNull)
			{
				return !rhs.Entity;
			}

			return lhs.Entity == rhs.Entity;
		}
	}

	public class TransformComponent : Component
	{
		public Vector3 Position
		{
			get
			{
				InternalCalls.TransformComponent_GetPosition(Entity.Id, out var position);
				return position;
			}

			set => InternalCalls.TransformComponent_SetPosition(Entity.Id, ref value);
		}

		public Vector3 Rotation
		{
			get
			{
				InternalCalls.TransformComponent_GetRotation(Entity.Id, out var rotation);
				return rotation;
			}

			set => InternalCalls.TransformComponent_SetRotation(Entity.Id, ref value);
		}

		public Vector3 Scale
		{
			get
			{
				InternalCalls.TransformComponent_GetScale(Entity.Id, out var scale);
				return scale;
			}

			set => InternalCalls.TransformComponent_SetScale(Entity.Id, ref value);
		}
	}

	public class SpriteRendererComponent : Component
	{
		public Color Color
		{
			get
			{
				InternalCalls.SpriteRendererComponent_GetColor(Entity.Id, out var color);
				return color;
			}

			set => InternalCalls.SpriteRendererComponent_SetColor(Entity.Id, ref value);
		}

		public Vector2 Tiling
		{
			get
			{
				InternalCalls.SpriteRendererComponent_GetTiling(Entity.Id, out var tiling);
				return tiling;
			}

			set => InternalCalls.SpriteRendererComponent_SetTiling(Entity.Id, ref value);
		}
	}

	public class CircleRendererComponent : Component
	{
		public Color Color
		{
			get
			{
				InternalCalls.CircleRendererComponent_GetColor(Entity.Id, out var color);
				return color;
			}

			set => InternalCalls.CircleRendererComponent_SetColor(Entity.Id, ref value);
		}

		public float Thickness
		{
			get
			{
				InternalCalls.CircleRendererComponent_GetThickness(Entity.Id, out var thickness);
				return thickness;
			}

			set => InternalCalls.CircleRendererComponent_SetThickness(Entity.Id, ref value);
		}

		public float Fade
		{
			get
			{
				InternalCalls.CircleRendererComponent_GetFade(Entity.Id, out var fade);
				return fade;
			}

			set => InternalCalls.CircleRendererComponent_SetFade(Entity.Id, ref value);
		}
	}

	public class Rigidbody2DComponent : Component
	{
		public void ApplyLinearImpulse(Vector2 impulse, Vector2 worldPoint, bool wake = true)
		{
			InternalCalls.Rigidbody2DComponent_ApplyLinearImpulse(Entity.Id, ref impulse, ref worldPoint, wake);
		}

		public void ApplyLinearImpulse(Vector2 impulse, bool wake = true)
		{
			InternalCalls.Rigidbody2DComponent_ApplyLinearImpulseToCenter(Entity.Id, ref impulse, wake);
		}

		public void ApplyAngularImpulse(float impulse, bool wake = true)
		{
			InternalCalls.Rigidbody2DComponent_ApplyAngularImpulse(Entity.Id, ref impulse, wake);
		}
	}

	public class AudioListenerComponent : Component
	{
		public bool IsVisibleInGame
		{

			get
			{
				InternalCalls.AudioListenerComponent_GetIsVisibleInGame(Entity.Id, out var isVisibleInGame);
				return isVisibleInGame;
			}

			set => InternalCalls.AudioListenerComponent_SetIsVisibleInGame(Entity.Id, ref value);
		}
	}
}
