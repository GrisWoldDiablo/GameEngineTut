#include "hzpch.h"
#include "Hazel.h"
#include "ScriptGlue.h"
#include "ScriptEngine.h"

#include "mono/metadata/object.h"
#include "mono/metadata/reflection.h"

#include "box2d/b2_body.h"

namespace Hazel
{
#define HZ_ADD_INTERNAL_CALL(Name) mono_add_internal_call("Hazel.InternalCalls::" #Name, Name)

	static std::unordered_map<MonoType*, std::function<bool(Entity)>> sEntityHasComponentFuncs;
	static std::unordered_map<MonoType*, std::function<void(Entity)>> sEntityAddComponentFuncs;

	/////////////////
	/// Inputs
	/////////////////

#pragma region Inputs
	static bool Input_IsKeyDown(KeyCode keyCode)
	{
		return Input::IsKeyPressed(keyCode);
	}
#pragma endregion

	/////////////////
	/// Entity
	/////////////////

#pragma region Entity
	static bool Entity_IsValid(UUID entityId)
	{
		auto* scene = ScriptEngine::GetSceneContext();
		HZ_CORE_ASSERT(scene, "Scene is null!");

		return scene->GetEntityByUUID(entityId);
	}

	static void Entity_Create(MonoString** name, UUID* outId)
	{
		auto* scene = ScriptEngine::GetSceneContext();
		HZ_CORE_ASSERT(scene, "Scene is null!");

		auto newEntity = scene->CreateEntity(mono_string_to_utf8(*name));

		*outId = newEntity.GetUUID();
	}

	static bool Entity_Destroy(UUID entityId)
	{
		auto* scene = ScriptEngine::GetSceneContext();
		HZ_CORE_ASSERT(scene, "Scene is null!");
		
		if (auto entity = scene->GetEntityByUUID(entityId))
		{
			scene->DestroyEntity(entity);
			return true;
		}

		return false;
	}

	static bool Entity_FindByName(MonoString** name, UUID* outId)
	{
		auto* scene = ScriptEngine::GetSceneContext();
		HZ_CORE_ASSERT(scene, "Scene is null!");

		*outId = UUID::Invalid();

		if (auto foundEntity = scene->GetEntityByName(mono_string_to_utf8(*name)))
		{
			*outId = foundEntity.GetUUID();
			return true;
		}

		return false;
	}

	static void Entity_AddComponent(UUID entityId, MonoReflectionType* componentType)
	{
		auto* scene = ScriptEngine::GetSceneContext();
		HZ_CORE_ASSERT(scene, "Scene is null!");
		auto entity = scene->GetEntityByUUID(entityId);
		HZ_CORE_ASSERT(entity, "Entity is null!");

		auto* managedType = mono_reflection_type_get_type(componentType);
		HZ_CORE_ASSERT(sEntityAddComponentFuncs.find(managedType) != sEntityAddComponentFuncs.end(), "");

		sEntityAddComponentFuncs.at(managedType)(entity);
	}

	static bool Entity_HasComponent(UUID entityId, MonoReflectionType* componentType)
	{
		auto* scene = ScriptEngine::GetSceneContext();
		HZ_CORE_ASSERT(scene, "Scene is null!");
		auto entity = scene->GetEntityByUUID(entityId);
		HZ_CORE_ASSERT(entity, "Entity is null!");

		auto* managedType = mono_reflection_type_get_type(componentType);
		HZ_CORE_ASSERT(sEntityHasComponentFuncs.find(managedType) != sEntityHasComponentFuncs.end(), "");

		return sEntityHasComponentFuncs.at(managedType)(entity);
	}

	static void Entity_GetName(UUID entityId, MonoString** outName)
	{
		auto* scene = ScriptEngine::GetSceneContext();
		HZ_CORE_ASSERT(scene, "Scene is null!");
		auto entity = scene->GetEntityByUUID(entityId);
		HZ_CORE_ASSERT(entity, "Entity is null!");

		*outName = mono_string_new_wrapper(entity.Name().c_str());
	}

	static void Entity_SetName(UUID entityId, MonoString** name)
	{
		auto* scene = ScriptEngine::GetSceneContext();
		HZ_CORE_ASSERT(scene, "Scene is null!");
		auto entity = scene->GetEntityByUUID(entityId);
		HZ_CORE_ASSERT(entity, "Entity is null!");

		entity.Name() = mono_string_to_utf8(*name);
	}
#pragma endregion

	/////////////////
	/// Components
	/////////////////

#pragma region Transform
	static void TransformComponent_GetPosition(UUID entityId, glm::vec3* outPosition)
	{
		auto* scene = ScriptEngine::GetSceneContext();
		HZ_CORE_ASSERT(scene, "Scene is null!");
		auto entity = scene->GetEntityByUUID(entityId);
		HZ_CORE_ASSERT(entity, "Entity is null!");

		*outPosition = entity.Transform().Position;
	}

	static void TransformComponent_SetPosition(UUID entityId, glm::vec3* position)
	{
		auto* scene = ScriptEngine::GetSceneContext();
		HZ_CORE_ASSERT(scene, "Scene is null!");
		auto entity = scene->GetEntityByUUID(entityId);
		HZ_CORE_ASSERT(entity, "Entity is null!");

		entity.Transform().Position = *position;
	}

	static void TransformComponent_GetRotation(UUID entityId, glm::vec3* outRotation)
	{
		auto* scene = ScriptEngine::GetSceneContext();
		HZ_CORE_ASSERT(scene, "Scene is null!");
		auto entity = scene->GetEntityByUUID(entityId);
		HZ_CORE_ASSERT(entity, "Entity is null!");

		*outRotation = glm::degrees(entity.Transform().Rotation);
	}

	static void TransformComponent_SetRotation(UUID entityId, glm::vec3* rotation)
	{
		auto* scene = ScriptEngine::GetSceneContext();
		HZ_CORE_ASSERT(scene, "Scene is null!");
		auto entity = scene->GetEntityByUUID(entityId);
		HZ_CORE_ASSERT(entity, "Entity is null!");

		entity.Transform().Rotation = glm::radians(*rotation);
	}

	static void TransformComponent_GetScale(UUID entityId, glm::vec3* outScale)
	{
		auto* scene = ScriptEngine::GetSceneContext();
		HZ_CORE_ASSERT(scene, "Scene is null!");
		auto entity = scene->GetEntityByUUID(entityId);
		HZ_CORE_ASSERT(entity, "Entity is null!");

		*outScale = entity.Transform().Scale;
	}

	static void TransformComponent_SetScale(UUID entityId, glm::vec3* scale)
	{
		auto* scene = ScriptEngine::GetSceneContext();
		HZ_CORE_ASSERT(scene, "Scene is null!");
		auto entity = scene->GetEntityByUUID(entityId);
		HZ_CORE_ASSERT(entity, "Entity is null!");

		entity.Transform().Scale = *scale;
	}
#pragma endregion

#pragma region SpriteRenderer

	// TODO Add sprite/texture class to C#

	static void SpriteRendererComponent_GetTiling(UUID entityId, glm::vec2* outTiling)
	{
		auto* scene = ScriptEngine::GetSceneContext();
		HZ_CORE_ASSERT(scene, "Scene is null!");
		auto entity = scene->GetEntityByUUID(entityId);
		HZ_CORE_ASSERT(entity, "Entity is null!");

		auto& component = entity.GetComponent<SpriteRendererComponent>();

		*outTiling = component.Tiling;
	}

	static void SpriteRendererComponent_SetTiling(UUID entityId, glm::vec3* tiling)
	{
		auto* scene = ScriptEngine::GetSceneContext();
		HZ_CORE_ASSERT(scene, "Scene is null!");
		auto entity = scene->GetEntityByUUID(entityId);
		HZ_CORE_ASSERT(entity, "Entity is null!");

		auto& component = entity.GetComponent<SpriteRendererComponent>();

		component.Tiling = *tiling;
	}

	static void SpriteRendererComponent_GetColor(UUID entityId, Color* outColor)
	{
		auto* scene = ScriptEngine::GetSceneContext();
		HZ_CORE_ASSERT(scene, "Scene is null!");
		auto entity = scene->GetEntityByUUID(entityId);
		HZ_CORE_ASSERT(entity, "Entity is null!");

		auto& component = entity.GetComponent<SpriteRendererComponent>();

		*outColor = component.Color;
	}

	static void SpriteRendererComponent_SetColor(UUID entityId, Color* color)
	{
		auto* scene = ScriptEngine::GetSceneContext();
		HZ_CORE_ASSERT(scene, "Scene is null!");
		auto entity = scene->GetEntityByUUID(entityId);
		HZ_CORE_ASSERT(entity, "Entity is null!");

		auto& component = entity.GetComponent<SpriteRendererComponent>();

		component.Color = *color;
	}
#pragma endregion

#pragma region CircleRenderer

	static void CircleRendererComponent_GetColor(UUID entityId, Color* outColor)
	{
		auto* scene = ScriptEngine::GetSceneContext();
		HZ_CORE_ASSERT(scene, "Scene is null!");
		auto entity = scene->GetEntityByUUID(entityId);
		HZ_CORE_ASSERT(entity, "Entity is null!");

		auto& component = entity.GetComponent<CircleRendererComponent>();

		*outColor = component.Color;
	}

	static void CircleRendererComponent_SetColor(UUID entityId, Color* color)
	{
		auto* scene = ScriptEngine::GetSceneContext();
		HZ_CORE_ASSERT(scene, "Scene is null!");
		auto entity = scene->GetEntityByUUID(entityId);
		HZ_CORE_ASSERT(entity, "Entity is null!");

		auto& component = entity.GetComponent<CircleRendererComponent>();

		component.Color = *color;
	}

	static void CircleRendererComponent_GetThickness(UUID entityId, float* outThickness)
	{
		auto* scene = ScriptEngine::GetSceneContext();
		HZ_CORE_ASSERT(scene, "Scene is null!");
		auto entity = scene->GetEntityByUUID(entityId);
		HZ_CORE_ASSERT(entity, "Entity is null!");

		auto& component = entity.GetComponent<CircleRendererComponent>();

		*outThickness = component.Thickness;
	}

	static void CircleRendererComponent_SetThickness(UUID entityId, float* thickness)
	{
		auto* scene = ScriptEngine::GetSceneContext();
		HZ_CORE_ASSERT(scene, "Scene is null!");
		auto entity = scene->GetEntityByUUID(entityId);
		HZ_CORE_ASSERT(entity, "Entity is null!");

		auto& component = entity.GetComponent<CircleRendererComponent>();

		component.Thickness = glm::clamp(*thickness, 0.0f, 1.0f);
	}

	static void CircleRendererComponent_GetFade(UUID entityId, float* outFade)
	{
		auto* scene = ScriptEngine::GetSceneContext();
		HZ_CORE_ASSERT(scene, "Scene is null!");
		auto entity = scene->GetEntityByUUID(entityId);
		HZ_CORE_ASSERT(entity, "Entity is null!");

		auto& component = entity.GetComponent<CircleRendererComponent>();

		*outFade = component.Fade;
	}

	static void CircleRendererComponent_SetFade(UUID entityId, float* fade)
	{
		auto* scene = ScriptEngine::GetSceneContext();
		HZ_CORE_ASSERT(scene, "Scene is null!");
		auto entity = scene->GetEntityByUUID(entityId);
		HZ_CORE_ASSERT(entity, "Entity is null!");

		auto& component = entity.GetComponent<CircleRendererComponent>();

		component.Fade = glm::max(0.0f, *fade);
	}
#pragma endregion

#pragma region Rigibody2D
	static void Rigidbody2DComponent_ApplyLinearImpulse(UUID entityId, glm::vec2* impulse, glm::vec2* worldPoint, bool wake)
	{
		auto* scene = ScriptEngine::GetSceneContext();
		HZ_CORE_ASSERT(scene, "Scene is null!");
		auto entity = scene->GetEntityByUUID(entityId);
		HZ_CORE_ASSERT(entity, "Entity is null!");

		auto& component = entity.GetComponent<Rigidbody2DComponent>();
		auto* body = static_cast<b2Body*>(component.RuntimeBody);

		body->ApplyLinearImpulse(b2Vec2(impulse->x, impulse->y), b2Vec2(worldPoint->x, worldPoint->y), wake);
	}

	static void Rigidbody2DComponent_ApplyLinearImpulseToCenter(UUID entityId, glm::vec2* impulse, bool wake)
	{
		auto* scene = ScriptEngine::GetSceneContext();
		HZ_CORE_ASSERT(scene, "Scene is null!");
		auto entity = scene->GetEntityByUUID(entityId);
		HZ_CORE_ASSERT(entity, "Entity is null!");

		auto& component = entity.GetComponent<Rigidbody2DComponent>();
		auto* body = static_cast<b2Body*>(component.RuntimeBody);

		body->ApplyLinearImpulseToCenter(b2Vec2(impulse->x, impulse->y), wake);
	}

	static void Rigidbody2DComponent_ApplyAngularImpulse(UUID entityId, float* impulse, bool wake)
	{
		auto* scene = ScriptEngine::GetSceneContext();
		HZ_CORE_ASSERT(scene, "Scene is null!");
		auto entity = scene->GetEntityByUUID(entityId);
		HZ_CORE_ASSERT(entity, "Entity is null!");

		auto& component = entity.GetComponent<Rigidbody2DComponent>();
		auto* body = static_cast<b2Body*>(component.RuntimeBody);

		body->ApplyAngularImpulse(*impulse, wake);
	}
#pragma endregion

#pragma region AudioListener
	static void AudioListenerComponent_GetPosition(UUID entityId, glm::vec3* outPosition)
	{
		auto* scene = ScriptEngine::GetSceneContext();
		HZ_CORE_ASSERT(scene, "Scene is null!");
		auto entity = scene->GetEntityByUUID(entityId);
		HZ_CORE_ASSERT(entity, "Entity is null!");

		// Will crash if entity does not have component.
		auto& component = entity.GetComponent<AudioListenerComponent>();

		*outPosition = entity.Transform().Position;
	}

	static void AudioListenerComponent_SetPosition(UUID entityId, glm::vec3* position)
	{
		auto* scene = ScriptEngine::GetSceneContext();
		HZ_CORE_ASSERT(scene, "Scene is null!");
		auto entity = scene->GetEntityByUUID(entityId);
		HZ_CORE_ASSERT(entity, "Entity is null!");

		auto& component = entity.GetComponent<AudioListenerComponent>();

		entity.Transform().Position = *position;
		component.SetPosition(*position);
	}

	static void AudioListenerComponent_GetIsVisibleInGame(UUID entityId, bool* outIsVisibleInGame)
	{
		auto* scene = ScriptEngine::GetSceneContext();
		HZ_CORE_ASSERT(scene, "Scene is null!");
		auto entity = scene->GetEntityByUUID(entityId);
		HZ_CORE_ASSERT(entity, "Entity is null!");

		// Will crash if entity does not have component.
		auto& component = entity.GetComponent<AudioListenerComponent>();

		*outIsVisibleInGame = component.IsVisibleInGame;
	}

	static void AudioListenerComponent_SetIsVisibleInGame(UUID entityId, bool* isVisibleInGame)
	{
		auto* scene = ScriptEngine::GetSceneContext();
		HZ_CORE_ASSERT(scene, "Scene is null!");
		auto entity = scene->GetEntityByUUID(entityId);
		HZ_CORE_ASSERT(entity, "Entity is null!");

		// Will crash if entity does not have component.
		auto& component = entity.GetComponent<AudioListenerComponent>();
		component.IsVisibleInGame = *isVisibleInGame;
	}
#pragma endregion

	/////////////////
	/// Registers
	/////////////////

	template<typename... TComponent>
	void ScriptGlue::RegisterComponent()
	{
		([]()
		{
			std::string_view typeName = typeid(TComponent).name();
			size_t pos = typeName.find_last_of(':');
			std::string_view componentName = typeName.substr(pos + 1);
			std::string managedTypeName = fmt::format("Hazel.{0}", componentName);

			auto* managedType = mono_reflection_type_from_name(managedTypeName.data(), ScriptEngine::GetCoreAssemblyImage());
			if (!managedType)
			{
				HZ_CORE_LERROR("Component not found! {0}", managedTypeName);
				return;
			}

			sEntityHasComponentFuncs[managedType] = [](Entity entity) { return entity.HasComponent<TComponent>(); };
			sEntityAddComponentFuncs[managedType] = [](Entity entity) { entity.AddComponent<TComponent>(); };
		}(), ...);
	}

	template<typename... TComponent>
	void ScriptGlue::RegisterComponents(ComponentGroup<TComponent...>)
	{
		RegisterComponent<TComponent...>();
	}

	void ScriptGlue::RegisterComponents()
	{
		sEntityHasComponentFuncs.clear();
		sEntityAddComponentFuncs.clear();
		RegisterComponents(AllComponents{});
	}

	void ScriptGlue::RegisterFunctions()
	{
		// Inputs
		HZ_ADD_INTERNAL_CALL(Input_IsKeyDown);

		// Entity
		HZ_ADD_INTERNAL_CALL(Entity_IsValid);
		HZ_ADD_INTERNAL_CALL(Entity_Create);
		HZ_ADD_INTERNAL_CALL(Entity_Destroy);
		HZ_ADD_INTERNAL_CALL(Entity_FindByName);
		HZ_ADD_INTERNAL_CALL(Entity_AddComponent);
		HZ_ADD_INTERNAL_CALL(Entity_HasComponent);
		HZ_ADD_INTERNAL_CALL(Entity_GetName);
		HZ_ADD_INTERNAL_CALL(Entity_SetName);

		// Transform
		HZ_ADD_INTERNAL_CALL(TransformComponent_GetPosition);
		HZ_ADD_INTERNAL_CALL(TransformComponent_SetPosition);
		HZ_ADD_INTERNAL_CALL(TransformComponent_GetRotation);
		HZ_ADD_INTERNAL_CALL(TransformComponent_SetRotation);
		HZ_ADD_INTERNAL_CALL(TransformComponent_GetScale);
		HZ_ADD_INTERNAL_CALL(TransformComponent_SetScale);

		// Sprite Renderer
		HZ_ADD_INTERNAL_CALL(SpriteRendererComponent_GetTiling);
		HZ_ADD_INTERNAL_CALL(SpriteRendererComponent_SetTiling);
		HZ_ADD_INTERNAL_CALL(SpriteRendererComponent_GetColor);
		HZ_ADD_INTERNAL_CALL(SpriteRendererComponent_SetColor);

		// Circle Renderer
		HZ_ADD_INTERNAL_CALL(CircleRendererComponent_GetColor);
		HZ_ADD_INTERNAL_CALL(CircleRendererComponent_SetColor);
		HZ_ADD_INTERNAL_CALL(CircleRendererComponent_GetThickness);
		HZ_ADD_INTERNAL_CALL(CircleRendererComponent_SetThickness);
		HZ_ADD_INTERNAL_CALL(CircleRendererComponent_GetFade);
		HZ_ADD_INTERNAL_CALL(CircleRendererComponent_SetFade);

		// Rigidbody 2D
		HZ_ADD_INTERNAL_CALL(Rigidbody2DComponent_ApplyLinearImpulse);
		HZ_ADD_INTERNAL_CALL(Rigidbody2DComponent_ApplyLinearImpulseToCenter);
		HZ_ADD_INTERNAL_CALL(Rigidbody2DComponent_ApplyAngularImpulse);

		// Audio Listener
		HZ_ADD_INTERNAL_CALL(AudioListenerComponent_GetPosition);
		HZ_ADD_INTERNAL_CALL(AudioListenerComponent_SetPosition);
		HZ_ADD_INTERNAL_CALL(AudioListenerComponent_GetIsVisibleInGame);
		HZ_ADD_INTERNAL_CALL(AudioListenerComponent_SetIsVisibleInGame);
	}
}
