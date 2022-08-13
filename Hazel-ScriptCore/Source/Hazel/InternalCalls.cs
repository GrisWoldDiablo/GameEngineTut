using System;
using System.Runtime.CompilerServices;

namespace Hazel
{
	internal class InternalCalls
	{
		//////////////
		// Inputs
		//////////////

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static bool Input_IsKeyDown(KeyCode keyCode);

		//////////////
		// Entities
		//////////////

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static bool Entity_HasComponent(ulong entityId, Type componentType);

		/* Transform */
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void TransformComponent_GetPosition(ulong entityId, out Vector3 position);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void TransformComponent_SetPosition(ulong entityId, ref Vector3 position);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void TransformComponent_GetRotation(ulong id, out Vector3 rotation);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void TransformComponent_SetRotation(ulong id, ref Vector3 value);

		/* Rigibody 2D */
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void Rigidbody2DComponent_ApplyLinearImpulse(ulong entityId, ref Vector2 impulse, ref Vector2 point, bool wake);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void Rigidbody2DComponent_ApplyLinearImpulseToCenter(ulong entityId, ref Vector2 impulse, bool wake);
	}
}
