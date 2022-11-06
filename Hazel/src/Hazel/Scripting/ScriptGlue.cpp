#include "hzpch.h"
#include "Hazel.h"
#include "ScriptGlue.h"
#include "ScriptEngine.h"
#include "ScriptInstance.h"

#include "Hazel/Audio/AudioEngine.h"

#include "mono/metadata/object.h"
#include "mono/metadata/reflection.h"

#include "box2d/b2_body.h"

namespace Hazel
{
#define HZ_ADD_INTERNAL_CALL(Name) mono_add_internal_call("Hazel.InternalCalls::" #Name, Name)

	static std::unordered_map<MonoType*, std::function<bool(Entity)>> sEntityHasComponentFuncs;
	static std::unordered_map<MonoType*, std::function<void(Entity)>> sEntityAddComponentFuncs;

	/////////////////
	/// Logger
	/////////////////

#pragma region Logger
	static void Logger_Log(MonoString* message)
	{
		auto* logMessage = mono_string_to_utf8(message);
		HZ_LDEBUG("[C#]: {0}", logMessage);
		mono_free(logMessage);
	}
#pragma endregion

	/////////////////
	/// Inputs
	/////////////////

#pragma region Inputs
#pragma region Keyboard
	static bool Input_IsKeyPressed(KeyCode keyCode)
	{
		return Input::IsKeyPressed(keyCode);
	}

	static bool Input_IsKeyDown(KeyCode keyCode)
	{
		return Input::IsKeyDown(keyCode);
	}

	static bool Input_IsKeyUp(KeyCode keyCode)
	{
		return Input::IsKeyUp(keyCode);
	}
#pragma endregion

#pragma region Mouse
	static bool Input_IsMouseButtonPressed(MouseCode mouseCode)
	{
		return Input::IsMouseButtonPressed(mouseCode);
	}

	static bool Input_IsMouseButtonDown(MouseCode mouseCode)
	{
		return Input::IsMouseButtonDown(mouseCode);
	}

	static bool Input_IsMouseButtonUp(MouseCode mouseCode)
	{
		return Input::IsMouseButtonUp(mouseCode);
	}
#pragma endregion
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

	static void Entity_Create(MonoString* name, MonoObject** outEntity)
	{
		auto* scene = ScriptEngine::GetSceneContext();
		HZ_CORE_ASSERT(scene, "Scene is null!");
		HZ_CORE_ASSERT(name, "Name is null!");

		*outEntity = nullptr;

		auto* entityName = mono_string_to_utf8(name);
		const auto newEntity = scene->CreateEntity(entityName);
		mono_free(entityName);

		if (const auto& entityScriptInstance = ScriptEngine::OnCreateEntity(newEntity))
		{
			*outEntity = entityScriptInstance->GetInstance();
		}
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

	static bool Entity_FindByName(MonoString* name, MonoObject** outEntity)
	{
		auto* scene = ScriptEngine::GetSceneContext();
		HZ_CORE_ASSERT(scene, "Scene is null!");
		HZ_CORE_ASSERT(name, "Name is null!");

		*outEntity = nullptr;
		auto* entityName = mono_string_to_utf8(name);
		auto foundEntity = scene->GetEntityByName(entityName);
		mono_free(entityName);

		if (foundEntity)
		{
			if (const auto& entityScriptInstance = ScriptEngine::GetEntityScriptInstance(foundEntity.GetUUID()))
			{
				*outEntity = entityScriptInstance->GetInstance();
				return true;
			}

			if (const auto& entityScriptInstance = ScriptEngine::OnCreateEntity(foundEntity))
			{
				*outEntity = entityScriptInstance->GetInstance();
				return true;
			}
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
		HZ_CORE_ASSERT(sEntityAddComponentFuncs.find(managedType) != sEntityAddComponentFuncs.end(), "Invalid Type!");

		sEntityAddComponentFuncs.at(managedType)(entity);
	}

	static bool Entity_HasComponent(UUID entityId, MonoReflectionType* componentType)
	{
		auto* scene = ScriptEngine::GetSceneContext();
		HZ_CORE_ASSERT(scene, "Scene is null!");
		auto entity = scene->GetEntityByUUID(entityId);
		HZ_CORE_ASSERT(entity, "Entity is null!");

		auto* managedType = mono_reflection_type_get_type(componentType);
		HZ_CORE_ASSERT(sEntityHasComponentFuncs.find(managedType) != sEntityHasComponentFuncs.end(), "Invalid Type!");

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

	static void Entity_SetName(UUID entityId, MonoString* name)
	{
		auto* scene = ScriptEngine::GetSceneContext();
		HZ_CORE_ASSERT(scene, "Scene is null!");
		auto entity = scene->GetEntityByUUID(entityId);
		HZ_CORE_ASSERT(entity, "Entity is null!");

		auto* entityName = mono_string_to_utf8(name);
		entity.Name() = entityName;
		mono_free(entityName);
	}
#pragma endregion

	/////////////////
	/// Components
	/////////////////

#pragma region Components
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

		if (entity.HasComponent<AudioListenerComponent>())
		{
			AudioEngine::SetListenerPosition(*position);
		}

		if (entity.HasComponent<AudioSourceComponent>())
		{
			auto& component = entity.GetComponent<AudioSourceComponent>();
			if (auto& source = component.AudioSource)
			{
				if (source->Get3D())
				{
					source->SetPosition(*position);
				}
			}
		}
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

	static void Rigidbody2DComponent_ApplyAngularImpulse(UUID entityId, float impulse, bool wake)
	{
		auto* scene = ScriptEngine::GetSceneContext();
		HZ_CORE_ASSERT(scene, "Scene is null!");
		auto entity = scene->GetEntityByUUID(entityId);
		HZ_CORE_ASSERT(entity, "Entity is null!");

		auto& component = entity.GetComponent<Rigidbody2DComponent>();
		auto* body = static_cast<b2Body*>(component.RuntimeBody);

		body->ApplyAngularImpulse(impulse, wake);
	}
#pragma endregion

#pragma region AudioListener
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

	static void AudioListenerComponent_SetIsVisibleInGame(UUID entityId, bool isVisibleInGame)
	{
		auto* scene = ScriptEngine::GetSceneContext();
		HZ_CORE_ASSERT(scene, "Scene is null!");
		auto entity = scene->GetEntityByUUID(entityId);
		HZ_CORE_ASSERT(entity, "Entity is null!");

		// Will crash if entity does not have component.
		auto& component = entity.GetComponent<AudioListenerComponent>();
		component.IsVisibleInGame = isVisibleInGame;
	}
#pragma endregion

#pragma region AudioSource
	static void AudioSourceComponent_GetGain(UUID entityId, float* outGain)
	{
		auto* scene = ScriptEngine::GetSceneContext();
		HZ_CORE_ASSERT(scene, "Scene is null!");
		auto entity = scene->GetEntityByUUID(entityId);
		HZ_CORE_ASSERT(entity, "Entity is null!");

		// Will crash if entity does not have component.
		auto& component = entity.GetComponent<AudioSourceComponent>();
		HZ_CORE_ASSERT(component.AudioSource, "AudioSource is Null!");

		*outGain = component.AudioSource->GetGain();
	}

	static void AudioSourceComponent_SetGain(UUID entityId, float gain)
	{
		auto* scene = ScriptEngine::GetSceneContext();
		HZ_CORE_ASSERT(scene, "Scene is null!");
		auto entity = scene->GetEntityByUUID(entityId);
		HZ_CORE_ASSERT(entity, "Entity is null!");

		// Will crash if entity does not have component.
		auto& component = entity.GetComponent<AudioSourceComponent>();
		HZ_CORE_ASSERT(component.AudioSource, "AudioSource is Null!");

		component.AudioSource->SetGain(gain);
	}

	static void AudioSourceComponent_GetPitch(UUID entityId, float* outPitch)
	{
		auto* scene = ScriptEngine::GetSceneContext();
		HZ_CORE_ASSERT(scene, "Scene is null!");
		auto entity = scene->GetEntityByUUID(entityId);
		HZ_CORE_ASSERT(entity, "Entity is null!");

		// Will crash if entity does not have component.
		auto& component = entity.GetComponent<AudioSourceComponent>();
		HZ_CORE_ASSERT(component.AudioSource, "AudioSource is Null!");

		*outPitch = component.AudioSource->GetPitch();
	}

	static void AudioSourceComponent_SetPitch(UUID entityId, float pitch)
	{
		auto* scene = ScriptEngine::GetSceneContext();
		HZ_CORE_ASSERT(scene, "Scene is null!");
		auto entity = scene->GetEntityByUUID(entityId);
		HZ_CORE_ASSERT(entity, "Entity is null!");

		// Will crash if entity does not have component.
		auto& component = entity.GetComponent<AudioSourceComponent>();
		HZ_CORE_ASSERT(component.AudioSource, "AudioSource is Null!");

		component.AudioSource->SetPitch(pitch);
	}

	static void AudioSourceComponent_GetLoop(UUID entityId, bool* outIsLoop)
	{
		auto* scene = ScriptEngine::GetSceneContext();
		HZ_CORE_ASSERT(scene, "Scene is null!");
		auto entity = scene->GetEntityByUUID(entityId);
		HZ_CORE_ASSERT(entity, "Entity is null!");

		// Will crash if entity does not have component.
		auto& component = entity.GetComponent<AudioSourceComponent>();
		HZ_CORE_ASSERT(component.AudioSource, "AudioSource is Null!");

		*outIsLoop = component.AudioSource->GetLoop();
	}

	static void AudioSourceComponent_SetLoop(UUID entityId, bool loop)
	{
		auto* scene = ScriptEngine::GetSceneContext();
		HZ_CORE_ASSERT(scene, "Scene is null!");
		auto entity = scene->GetEntityByUUID(entityId);
		HZ_CORE_ASSERT(entity, "Entity is null!");

		// Will crash if entity does not have component.
		auto& component = entity.GetComponent<AudioSourceComponent>();
		HZ_CORE_ASSERT(component.AudioSource, "AudioSource is Null!");

		component.AudioSource->SetLoop(loop);
	}

	static void AudioSourceComponent_Get3D(UUID entityId, bool* outIs3D)
	{
		auto* scene = ScriptEngine::GetSceneContext();
		HZ_CORE_ASSERT(scene, "Scene is null!");
		auto entity = scene->GetEntityByUUID(entityId);
		HZ_CORE_ASSERT(entity, "Entity is null!");

		// Will crash if entity does not have component.
		auto& component = entity.GetComponent<AudioSourceComponent>();
		HZ_CORE_ASSERT(component.AudioSource, "AudioSource is Null!");

		*outIs3D = component.AudioSource->Get3D();
	}

	static void AudioSourceComponent_Set3D(UUID entityId, bool is3D)
	{
		auto* scene = ScriptEngine::GetSceneContext();
		HZ_CORE_ASSERT(scene, "Scene is null!");
		auto entity = scene->GetEntityByUUID(entityId);
		HZ_CORE_ASSERT(entity, "Entity is null!");

		// Will crash if entity does not have component.
		auto& component = entity.GetComponent<AudioSourceComponent>();
		HZ_CORE_ASSERT(component.AudioSource, "AudioSource is Null!");

		component.AudioSource->Set3D(is3D);
	}

	static void AudioSourceComponent_GetState(UUID entityId, int* outState)
	{
		auto* scene = ScriptEngine::GetSceneContext();
		HZ_CORE_ASSERT(scene, "Scene is null!");
		auto entity = scene->GetEntityByUUID(entityId);
		HZ_CORE_ASSERT(entity, "Entity is null!");

		// Will crash if entity does not have component.
		auto& component = entity.GetComponent<AudioSourceComponent>();
		HZ_CORE_ASSERT(component.AudioSource, "AudioSource is Null!");

		*outState = static_cast<int>(component.AudioSource->GetState());
	}

	static void AudioSourceComponent_GetOffset(UUID entityId, float* outOffset)
	{
		auto* scene = ScriptEngine::GetSceneContext();
		HZ_CORE_ASSERT(scene, "Scene is null!");
		auto entity = scene->GetEntityByUUID(entityId);
		HZ_CORE_ASSERT(entity, "Entity is null!");

		// Will crash if entity does not have component.
		auto& component = entity.GetComponent<AudioSourceComponent>();
		HZ_CORE_ASSERT(component.AudioSource, "AudioSource is Null!");

		*outOffset = component.AudioSource->GetOffset();
	}

	static void AudioSourceComponent_SetOffset(UUID entityId, float* offset)
	{
		auto* scene = ScriptEngine::GetSceneContext();
		HZ_CORE_ASSERT(scene, "Scene is null!");
		auto entity = scene->GetEntityByUUID(entityId);
		HZ_CORE_ASSERT(entity, "Entity is null!");

		// Will crash if entity does not have component.
		auto& component = entity.GetComponent<AudioSourceComponent>();
		HZ_CORE_ASSERT(component.AudioSource, "AudioSource is Null!");

		component.AudioSource->SetOffset(*offset);
	}

	static void AudioSourceComponent_GetLength(UUID entityId, float* outLenght)
	{
		auto* scene = ScriptEngine::GetSceneContext();
		HZ_CORE_ASSERT(scene, "Scene is null!");
		auto entity = scene->GetEntityByUUID(entityId);
		HZ_CORE_ASSERT(entity, "Entity is null!");

		// Will crash if entity does not have component.
		auto& component = entity.GetComponent<AudioSourceComponent>();
		HZ_CORE_ASSERT(component.AudioSource, "AudioSource is Null!");

		*outLenght = component.AudioSource->GetLength();
	}

	static void AudioSourceComponent_GetPath(UUID entityId, MonoString** outPath)
	{
		auto* scene = ScriptEngine::GetSceneContext();
		HZ_CORE_ASSERT(scene, "Scene is null!");
		auto entity = scene->GetEntityByUUID(entityId);
		HZ_CORE_ASSERT(entity, "Entity is null!");

		// Will crash if entity does not have component.
		auto& component = entity.GetComponent<AudioSourceComponent>();
		HZ_CORE_ASSERT(component.AudioSource, "AudioSource is Null!");

		*outPath = mono_string_new_wrapper(component.AudioSource->GetPath().string().c_str());
	}

	static void AudioSourceComponent_GetIsVisibleInGame(UUID entityId, bool* outIsVisibleInGame)
	{
		auto* scene = ScriptEngine::GetSceneContext();
		HZ_CORE_ASSERT(scene, "Scene is null!");
		auto entity = scene->GetEntityByUUID(entityId);
		HZ_CORE_ASSERT(entity, "Entity is null!");

		// Will crash if entity does not have component.
		auto& component = entity.GetComponent<AudioSourceComponent>();

		*outIsVisibleInGame = component.IsVisibleInGame;
	}

	static void AudioSourceComponent_SetIsVisibleInGame(UUID entityId, bool isVisibleInGame)
	{
		auto* scene = ScriptEngine::GetSceneContext();
		HZ_CORE_ASSERT(scene, "Scene is null!");
		auto entity = scene->GetEntityByUUID(entityId);
		HZ_CORE_ASSERT(entity, "Entity is null!");

		// Will crash if entity does not have component.
		auto& component = entity.GetComponent<AudioSourceComponent>();
		component.IsVisibleInGame = isVisibleInGame;
	}

	static void AudioSourceComponent_Play(UUID entityId)
	{
		auto* scene = ScriptEngine::GetSceneContext();
		HZ_CORE_ASSERT(scene, "Scene is null!");
		auto entity = scene->GetEntityByUUID(entityId);
		HZ_CORE_ASSERT(entity, "Entity is null!");

		// Will crash if entity does not have component.
		auto& component = entity.GetComponent<AudioSourceComponent>();
		HZ_CORE_ASSERT(component.AudioSource, "AudioSource is Null!");

		component.AudioSource->Play();
	}

	static void AudioSourceComponent_Stop(UUID entityId)
	{
		auto* scene = ScriptEngine::GetSceneContext();
		HZ_CORE_ASSERT(scene, "Scene is null!");
		auto entity = scene->GetEntityByUUID(entityId);
		HZ_CORE_ASSERT(entity, "Entity is null!");

		// Will crash if entity does not have component.
		auto& component = entity.GetComponent<AudioSourceComponent>();
		HZ_CORE_ASSERT(component.AudioSource, "AudioSource is Null!");

		component.AudioSource->Stop();
	}

	static void AudioSourceComponent_Pause(UUID entityId)
	{
		auto* scene = ScriptEngine::GetSceneContext();
		HZ_CORE_ASSERT(scene, "Scene is null!");
		auto entity = scene->GetEntityByUUID(entityId);
		HZ_CORE_ASSERT(entity, "Entity is null!");

		// Will crash if entity does not have component.
		auto& component = entity.GetComponent<AudioSourceComponent>();
		HZ_CORE_ASSERT(component.AudioSource, "AudioSource is Null!");

		component.AudioSource->Pause();
	}

	static void AudioSourceComponent_Rewind(UUID entityId)
	{
		auto* scene = ScriptEngine::GetSceneContext();
		HZ_CORE_ASSERT(scene, "Scene is null!");
		auto entity = scene->GetEntityByUUID(entityId);
		HZ_CORE_ASSERT(entity, "Entity is null!");

		// Will crash if entity does not have component.
		auto& component = entity.GetComponent<AudioSourceComponent>();
		HZ_CORE_ASSERT(component.AudioSource, "AudioSource is Null!");

		component.AudioSource->Rewind();
	}
#pragma endregion
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
#pragma region Logger
		HZ_ADD_INTERNAL_CALL(Logger_Log);
#pragma endregion

#pragma region Inputs
#pragma region Keyboard
		HZ_ADD_INTERNAL_CALL(Input_IsKeyPressed);
		HZ_ADD_INTERNAL_CALL(Input_IsKeyDown);
		HZ_ADD_INTERNAL_CALL(Input_IsKeyUp);
#pragma endregion

#pragma region Mouse
		HZ_ADD_INTERNAL_CALL(Input_IsMouseButtonPressed);
		HZ_ADD_INTERNAL_CALL(Input_IsMouseButtonDown);
		HZ_ADD_INTERNAL_CALL(Input_IsMouseButtonUp);
#pragma endregion
#pragma endregion

#pragma region Entity
		HZ_ADD_INTERNAL_CALL(Entity_IsValid);
		HZ_ADD_INTERNAL_CALL(Entity_Create);
		HZ_ADD_INTERNAL_CALL(Entity_Destroy);
		HZ_ADD_INTERNAL_CALL(Entity_FindByName);
		HZ_ADD_INTERNAL_CALL(Entity_AddComponent);
		HZ_ADD_INTERNAL_CALL(Entity_HasComponent);
		HZ_ADD_INTERNAL_CALL(Entity_GetName);
		HZ_ADD_INTERNAL_CALL(Entity_SetName);
#pragma endregion

#pragma region Transform
		HZ_ADD_INTERNAL_CALL(TransformComponent_GetPosition);
		HZ_ADD_INTERNAL_CALL(TransformComponent_SetPosition);
		HZ_ADD_INTERNAL_CALL(TransformComponent_GetRotation);
		HZ_ADD_INTERNAL_CALL(TransformComponent_SetRotation);
		HZ_ADD_INTERNAL_CALL(TransformComponent_GetScale);
		HZ_ADD_INTERNAL_CALL(TransformComponent_SetScale);
#pragma endregion

#pragma region Components
#pragma region Sprite Renderer
		HZ_ADD_INTERNAL_CALL(SpriteRendererComponent_GetTiling);
		HZ_ADD_INTERNAL_CALL(SpriteRendererComponent_SetTiling);
		HZ_ADD_INTERNAL_CALL(SpriteRendererComponent_GetColor);
		HZ_ADD_INTERNAL_CALL(SpriteRendererComponent_SetColor);
#pragma endregion

#pragma region Circle Renderer
		HZ_ADD_INTERNAL_CALL(CircleRendererComponent_GetColor);
		HZ_ADD_INTERNAL_CALL(CircleRendererComponent_SetColor);
		HZ_ADD_INTERNAL_CALL(CircleRendererComponent_GetThickness);
		HZ_ADD_INTERNAL_CALL(CircleRendererComponent_SetThickness);
		HZ_ADD_INTERNAL_CALL(CircleRendererComponent_GetFade);
		HZ_ADD_INTERNAL_CALL(CircleRendererComponent_SetFade);
#pragma endregion

#pragma region Rigidbody 2D
		HZ_ADD_INTERNAL_CALL(Rigidbody2DComponent_ApplyLinearImpulse);
		HZ_ADD_INTERNAL_CALL(Rigidbody2DComponent_ApplyLinearImpulseToCenter);
		HZ_ADD_INTERNAL_CALL(Rigidbody2DComponent_ApplyAngularImpulse);
#pragma endregion

#pragma region Audio Listener
		HZ_ADD_INTERNAL_CALL(AudioListenerComponent_GetIsVisibleInGame);
		HZ_ADD_INTERNAL_CALL(AudioListenerComponent_SetIsVisibleInGame);
#pragma endregion

#pragma region Audio Source
		HZ_ADD_INTERNAL_CALL(AudioSourceComponent_GetGain);
		HZ_ADD_INTERNAL_CALL(AudioSourceComponent_SetGain);
		HZ_ADD_INTERNAL_CALL(AudioSourceComponent_GetPitch);
		HZ_ADD_INTERNAL_CALL(AudioSourceComponent_SetPitch);
		HZ_ADD_INTERNAL_CALL(AudioSourceComponent_GetLoop);
		HZ_ADD_INTERNAL_CALL(AudioSourceComponent_SetLoop);
		HZ_ADD_INTERNAL_CALL(AudioSourceComponent_Get3D);
		HZ_ADD_INTERNAL_CALL(AudioSourceComponent_Set3D);
		HZ_ADD_INTERNAL_CALL(AudioSourceComponent_GetState);
		HZ_ADD_INTERNAL_CALL(AudioSourceComponent_GetOffset);
		HZ_ADD_INTERNAL_CALL(AudioSourceComponent_SetOffset);
		HZ_ADD_INTERNAL_CALL(AudioSourceComponent_GetLength);
		HZ_ADD_INTERNAL_CALL(AudioSourceComponent_GetPath);
		HZ_ADD_INTERNAL_CALL(AudioSourceComponent_GetIsVisibleInGame);
		HZ_ADD_INTERNAL_CALL(AudioSourceComponent_SetIsVisibleInGame);
		HZ_ADD_INTERNAL_CALL(AudioSourceComponent_Play);
		HZ_ADD_INTERNAL_CALL(AudioSourceComponent_Stop);
		HZ_ADD_INTERNAL_CALL(AudioSourceComponent_Pause);
		HZ_ADD_INTERNAL_CALL(AudioSourceComponent_Rewind);
#pragma endregion
#pragma endregion
	}
}
