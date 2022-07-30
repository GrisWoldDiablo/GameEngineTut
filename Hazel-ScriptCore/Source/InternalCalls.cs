using System.Runtime.CompilerServices;

namespace Hazel
{
	public static class InternalCalls
	{
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void NativeLog(string text, int parameter);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void NativeLog_Vector3(ref Vector3 position, out Vector3 result);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static float NativeLog_Vector3Dot(ref Vector3 position);

		//////////////
		// Entity
		//////////////

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void Entity_GetPosition(ulong entityId, out Vector3 position);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void Entity_SetPosition(ulong entityId, ref Vector3 position);

		//////////////
		// Inputs
		//////////////

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static bool Input_IsKeyDown(KeyCode keyCode);

		// Returning struct might be bad for memory.k
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static Vector3 NativeLog_Vector3Struct(ref Vector3 position);
	}
}
