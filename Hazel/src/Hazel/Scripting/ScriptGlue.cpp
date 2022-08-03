#include "hzpch.h"
#include "ScriptGlue.h"
#include "ScriptEngine.h"

#include "Hazel.h"

#include "mono/metadata/object.h"
#include "mono/metadata/reflection.h"


namespace Hazel
{
#define HZ_ADD_INTERNAL_CALL(Name) mono_add_internal_call("Hazel.InternalCalls::" #Name, Name)

	static std::unordered_map<MonoType*, std::function<bool(Entity)>> sEntityHasComponentFuncs;
	
	/////////////////
	/// Entity
	/////////////////

	static bool Entity_HasComponent(UUID entityId, MonoReflectionType* componentType)
	{
		auto* scene = ScriptEngine::GetSceneContext();
		HZ_CORE_ASSERT(scene, "Scene is null");
		auto entity = scene->GetEntityByUUID(entityId);
		HZ_CORE_ASSERT(entity, "Entity is null");

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
		auto entity = scene->GetEntityByUUID(entityId);
		*outPosition = entity.Transform().Position;
	}

	static void TransformComponent_SetPosition(UUID entityId, glm::vec3* position)
	{
		auto* scene = ScriptEngine::GetSceneContext();
		auto entity = scene->GetEntityByUUID(entityId);
		entity.Transform().Position = *position;
	}

	/////////////////
	/// Inputs
	/////////////////

	static bool Input_IsKeyDown(KeyCode keyCode)
	{
		return Input::IsKeyPressed(keyCode);
	}

	/////////////////
	/// Registers
	/////////////////

	template<typename TComponent>
	void ScriptGlue::RegisterComponent()
	{
		std::string_view typeName = typeid(TComponent).name();
		size_t pos = typeName.find_last_of(':');
		std::string_view componentName = typeName.substr(pos + 1);
		std::string managedTypeName = fmt::format("Hazel.{0}", componentName);

		auto* managedType = mono_reflection_type_from_name(managedTypeName.data(), ScriptEngine::GetCoreAssemblyImage());
		HZ_CORE_ASSERT(managedType, "Type not found in assembly!");
		sEntityHasComponentFuncs[managedType] = [](Entity entity) { return entity.HasComponent<TComponent>(); };
	}

	void ScriptGlue::RegisterComponents()
	{
		RegisterComponent<TransformComponent>();
	}

	void ScriptGlue::RegisterFunctions()
	{
		HZ_ADD_INTERNAL_CALL(Entity_HasComponent);

		HZ_ADD_INTERNAL_CALL(TransformComponent_GetPosition);
		HZ_ADD_INTERNAL_CALL(TransformComponent_SetPosition);

		HZ_ADD_INTERNAL_CALL(Input_IsKeyDown);
	}
}
