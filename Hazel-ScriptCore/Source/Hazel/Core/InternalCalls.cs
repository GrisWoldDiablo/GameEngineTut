﻿using System;
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
		// Entity
		//////////////

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static bool Entity_IsValid(ulong entityId);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void Entity_Create(ref string name, out Entity entity);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static bool Entity_Destroy(ulong entityId);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static bool Entity_FindByName(ref string name, out Entity entity);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void Entity_AddComponent(ulong entityId, Type type);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static bool Entity_HasComponent(ulong entityId, Type componentType);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void Entity_GetName(ulong entityId, out string name);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void Entity_SetName(ulong entityId, ref string name);

		//////////////
		// Components
		//////////////

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

		/* Sprite Renderer */
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void SpriteRendererComponent_GetTiling(ulong entityId, out Vector2 tiling);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void SpriteRendererComponent_SetTiling(ulong entityId, ref Vector2 tiling);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void SpriteRendererComponent_GetColor(ulong entityId, out Color color);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void SpriteRendererComponent_SetColor(ulong entityId, ref Color color);

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

		/* Rigibody 2D */
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void Rigidbody2DComponent_ApplyLinearImpulse(ulong entityId, ref Vector2 impulse, ref Vector2 point, bool wake);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void Rigidbody2DComponent_ApplyLinearImpulseToCenter(ulong entityId, ref Vector2 impulse, bool wake);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void Rigidbody2DComponent_ApplyAngularImpulse(ulong entityId, ref float impulse, bool wake);

		/* AudioListener */
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void AudioListenerComponent_GetIsVisibleInGame(ulong entityId, out bool isVisibleInGame);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void AudioListenerComponent_SetIsVisibleInGame(ulong entityId, ref bool isVisibleInGame);
	}
}
