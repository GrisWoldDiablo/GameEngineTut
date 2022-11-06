using System;
using System.Runtime.CompilerServices;

namespace Hazel
{
	internal class InternalCalls
	{
		//////////////
		// Logger
		//////////////

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void Logger_Log(string message);

		//////////////
		// Inputs
		//////////////

		#region Keyboard
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static bool Input_IsKeyPressed(KeyCode keyCode);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static bool Input_IsKeyDown(KeyCode keyCode);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static bool Input_IsKeyUp(KeyCode keyCode);
		#endregion

		#region Mouse
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static bool Input_IsMouseButtonPressed(MouseCode mouseCode);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static bool Input_IsMouseButtonDown(MouseCode mouseCode);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static bool Input_IsMouseButtonUp(MouseCode mouseCode);
		#endregion

		//////////////
		// Entity
		//////////////

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static bool Entity_IsValid(ulong entityId);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void Entity_Create(string name, out Entity entity);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static bool Entity_Destroy(ulong entityId);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static bool Entity_FindByName(string name, out Entity entity);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void Entity_AddComponent(ulong entityId, Type type);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static bool Entity_HasComponent(ulong entityId, Type componentType);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void Entity_GetName(ulong entityId, out string name);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void Entity_SetName(ulong entityId, string name);

		//////////////
		// Components
		//////////////

		#region Transform
		/* Transform */
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void TransformComponent_GetPosition(ulong entityId, out Vector3 position);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void TransformComponent_SetPosition(ulong entityId, ref Vector3 position);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void TransformComponent_GetRotation(ulong entityId, out Vector3 rotation);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void TransformComponent_SetRotation(ulong entityId, ref Vector3 rotation);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void TransformComponent_GetScale(ulong entityId, out Vector3 scale);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void TransformComponent_SetScale(ulong entityId, ref Vector3 scale);
		#endregion

		#region Sprite Renderer
		/* Sprite Renderer */
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void SpriteRendererComponent_GetTiling(ulong entityId, out Vector2 tiling);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void SpriteRendererComponent_SetTiling(ulong entityId, ref Vector2 tiling);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void SpriteRendererComponent_GetColor(ulong entityId, out Color color);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void SpriteRendererComponent_SetColor(ulong entityId, ref Color color);
		#endregion

		#region Circle Renderer
		/* Circle Renderer*/
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void CircleRendererComponent_GetColor(ulong entityId, out Color color);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void CircleRendererComponent_SetColor(ulong entityId, ref Color color);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void CircleRendererComponent_GetThickness(ulong entityId, out float thickness);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void CircleRendererComponent_SetThickness(ulong entityId, ref float thickness);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void CircleRendererComponent_GetFade(ulong entityId, out float fade);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void CircleRendererComponent_SetFade(ulong entityId, ref float fade);
		#endregion

		#region Rigibody 2D
		/* Rigibody 2D */
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void Rigidbody2DComponent_ApplyLinearImpulse(ulong entityId, ref Vector2 impulse, ref Vector2 point, bool wake);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void Rigidbody2DComponent_ApplyLinearImpulseToCenter(ulong entityId, ref Vector2 impulse, bool wake);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void Rigidbody2DComponent_ApplyAngularImpulse(ulong entityId, float impulse, bool wake);
		#endregion

		#region Audio Listener
		/* Audio Listener */
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void AudioListenerComponent_GetIsVisibleInGame(ulong entityId, out bool isVisibleInGame);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void AudioListenerComponent_SetIsVisibleInGame(ulong entityId, bool isVisibleInGame);
		#endregion

		#region Audio Source
		/* Audio Source */
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void AudioSourceComponent_GetGain(UUID id, out float gain);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void AudioSourceComponent_SetGain(UUID id, float gain);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void AudioSourceComponent_GetPitch(UUID id, out float pitch);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void AudioSourceComponent_SetPitch(UUID id, float pitch);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void AudioSourceComponent_GetLoop(UUID id, out bool isLoop);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void AudioSourceComponent_SetLoop(UUID id, bool isLoop);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void AudioSourceComponent_Get3D(UUID id, out bool is3D);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void AudioSourceComponent_Set3D(UUID id, bool is3D);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void AudioSourceComponent_GetState(UUID id, out int state);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void AudioSourceComponent_GetOffset(UUID id, out float offset);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void AudioSourceComponent_SetOffset(UUID id, float offset);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void AudioSourceComponent_GetLength(UUID id, out float lenght);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void AudioSourceComponent_GetPath(UUID id, out string path);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void AudioSourceComponent_Play(UUID id);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void AudioSourceComponent_Stop(UUID id);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void AudioSourceComponent_Pause(UUID id);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void AudioSourceComponent_Rewind(UUID id);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void AudioSourceComponent_GetIsVisibleInGame(UUID id, out bool isVisibleInGame);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void AudioSourceComponent_SetIsVisibleInGame(UUID id, bool isVisibleInGame);
		#endregion
	}
}
