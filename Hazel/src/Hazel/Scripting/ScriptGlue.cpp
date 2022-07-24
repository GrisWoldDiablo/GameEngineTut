#include "hzpch.h"
#include "ScriptGlue.h"

#include "mono/metadata/object.h"

namespace Hazel
{
#define HZ_ADD_INTERNAL_CALL(Name) mono_add_internal_call("Hazel.InternalCalls::" #Name, Name)

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

		mono_add_internal_call("Hazel.InternalCalls::NativeLog_Vector3Struct", NativeLog_Vector3Struct);
	}
}
