#include "hzpch.h"
#include "ScriptGlue.h"
#include "ScriptEngine.h"

#include "Hazel.h"

#include "mono/metadata/object.h"

#define HZ_ADD_INTERNAL_CALL(Name) mono_add_internal_call("Hazel.InternalCalls::" #Name, Name)

namespace Hazel
{
	static void NativeLog(MonoString* string, int parameter)
	{
		char* cStr = mono_string_to_utf8(string);
		std::string str(cStr);
		mono_free(cStr);
		HZ_CORE_LDEBUG("Comes from C# {0} : {1}", str, parameter);
	}

	static void NativeLog_Vector3(glm::vec3* parameter, glm::vec3* outResult)
	{
		HZ_CORE_LDEBUG("Comes from C# Vector3 {0}", *parameter);

		*outResult = glm::normalize(*parameter);
	}

	static float NativeLog_Vector3Dot(glm::vec3* parameter)
	{
		HZ_CORE_LDEBUG("Comes from C# Vector3Dot {0}", *parameter);

		return glm::dot(*parameter, *parameter);
	}

	static void Entity_GetPosition(UUID entityId, glm::vec3* outPosition)
	{
		auto* scene = ScriptEngine::GetSceneContext();
		Entity entity = scene->GetEntityByUUID(entityId);
		*outPosition = entity.Transform().Position;
	}

	static void Entity_SetPosition(UUID entityId, glm::vec3* position)
	{
		auto* scene = ScriptEngine::GetSceneContext();
		Entity entity = scene->GetEntityByUUID(entityId);
		entity.Transform().Position = *position;
	}

	static bool Input_IsKeyDown(KeyCode keyCode)
	{
		return Input::IsKeyPressed(keyCode);
	}

	// Returning struct might be bad for memory
	static glm::vec3 NativeLog_Vector3Struct(glm::vec3* parameter)
	{
		HZ_CORE_LDEBUG("Comes from C# Vector3Struct {0}", *parameter);

		return glm::normalize(*parameter);
	}

	void ScriptGlue::RegisterFunctions()
	{
		HZ_ADD_INTERNAL_CALL(NativeLog);
		HZ_ADD_INTERNAL_CALL(NativeLog_Vector3);
		HZ_ADD_INTERNAL_CALL(NativeLog_Vector3Dot);

		HZ_ADD_INTERNAL_CALL(Entity_GetPosition);
		HZ_ADD_INTERNAL_CALL(Entity_SetPosition);

		HZ_ADD_INTERNAL_CALL(Input_IsKeyDown);

		// Returning struct might be bad for memory
		HZ_ADD_INTERNAL_CALL(NativeLog_Vector3Struct);

		// Old ways
		//mono_add_internal_call("Hazel.InternalCalls::NativeLog_Vector3Struct", NativeLog_Vector3Struct);
	}
}
