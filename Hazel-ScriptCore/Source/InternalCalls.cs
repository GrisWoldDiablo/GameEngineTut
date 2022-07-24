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

		// Returning struct might be bad for memory.k
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static Vector3 NativeLog_Vector3Struct(ref Vector3 position);
	}
}
