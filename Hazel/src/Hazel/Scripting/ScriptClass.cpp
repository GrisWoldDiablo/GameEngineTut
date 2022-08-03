#include "hzpch.h"
#include "ScriptClass.h"
#include "ScriptEngine.h"

#include "mono/jit/jit.h"
#include "mono/metadata/assembly.h"
#include "mono/metadata/object.h"

namespace Hazel
{
	ScriptClass::ScriptClass(const std::string& classNamespace, const std::string& className)
		:_classNamespace(classNamespace), _className(className)
	{
		_monoClass = mono_class_from_name(ScriptEngine::GetCoreAssemblyImage(), classNamespace.c_str(), className.c_str());
	}

	MonoObject* ScriptClass::Instanciate(MonoMethod* constructor, void** params)
	{
		return ScriptEngine::InstanciateClass(_monoClass, constructor, params);
	}

	MonoMethod* ScriptClass::GetMethod(const std::string& name, int paramsCount)
	{
		// INFO Will remove on future commit.
		// Get Method names and params

		//int mcount = mono_class_num_methods(_monoClass);
		//void* iter = nullptr;
		//MonoMethod* class_method = nullptr;

		//while ((class_method = mono_class_get_methods(_monoClass, &iter)))
		//{
		//	MonoMethodSignature* sig = mono_method_signature(class_method);
		//	int count = mono_signature_get_param_count(sig);
		//	char* names[10];
		//	for (size_t i = 0; i < count; i++)
		//	{
		//		names[i] = new char[30]();
		//	}
		//	
		//	if (count > 0)
		//	{
		//		mono_method_get_param_names(class_method, (const char**)names);
		//	}
		//	std::stringstream params;
		//	for (size_t i = 0; i < count; i++)
		//	{
		//		params << ", " << names[i];
		//	}
		//	HZ_CORE_LINFO("Class Method [{0}({1})]", mono_method_get_name(class_method), params.str());
		//}

		return mono_class_get_method_from_name(_monoClass, name.c_str(), paramsCount);
	}

	MonoObject* ScriptClass::InvokeMethod(MonoObject* instance, MonoMethod* monoMethod, void** params)
	{
		return mono_runtime_invoke(monoMethod, instance, params, nullptr);
	}

}
