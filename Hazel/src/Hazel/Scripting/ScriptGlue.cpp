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

	/////////////////
	/// Inputs
	/////////////////

	static bool Input_IsKeyDown(KeyCode keyCode)
	{
		return Input::IsKeyPressed(keyCode);
	}

	/////////////////
	/// Entity
	/////////////////

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

	/////////////////
	/// Components
	/////////////////

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

	static void Rigidbody2DComponent_ApplyLinearImpulse(UUID entityId, glm::vec2* impulse, glm::vec2* worldPoint, bool wake)
	{
		auto* scene = ScriptEngine::GetSceneContext();
		HZ_CORE_ASSERT(scene, "Scene is null!");
		auto entity = scene->GetEntityByUUID(entityId);
		HZ_CORE_ASSERT(entity, "Entity is null!");

		auto& rb2d = entity.GetComponent<Rigidbody2DComponent>();
		auto* body = static_cast<b2Body*>(rb2d.RuntimeBody);

		body->ApplyLinearImpulse(b2Vec2(impulse->x, impulse->y), b2Vec2(worldPoint->x, worldPoint->y), wake);
	}

	static void Rigidbody2DComponent_ApplyLinearImpulseToCenter(UUID entityId, glm::vec2* impulse, bool wake)
	{
		auto* scene = ScriptEngine::GetSceneContext();
		HZ_CORE_ASSERT(scene, "Scene is null!");
		auto entity = scene->GetEntityByUUID(entityId);
		HZ_CORE_ASSERT(entity, "Entity is null!");

		auto& rb2d = entity.GetComponent<Rigidbody2DComponent>();
		auto* body = static_cast<b2Body*>(rb2d.RuntimeBody);

		body->ApplyLinearImpulseToCenter(b2Vec2(impulse->x, impulse->y), wake);
	}

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
		RegisterComponents(AllComponents{});
	}

	void ScriptGlue::RegisterFunctions()
	{
		HZ_ADD_INTERNAL_CALL(Input_IsKeyDown);

		HZ_ADD_INTERNAL_CALL(Entity_HasComponent);

		HZ_ADD_INTERNAL_CALL(TransformComponent_GetPosition);
		HZ_ADD_INTERNAL_CALL(TransformComponent_SetPosition);

		HZ_ADD_INTERNAL_CALL(Rigidbody2DComponent_ApplyLinearImpulse);
		HZ_ADD_INTERNAL_CALL(Rigidbody2DComponent_ApplyLinearImpulseToCenter);
	}
}
