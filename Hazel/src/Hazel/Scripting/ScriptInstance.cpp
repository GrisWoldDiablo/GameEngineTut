#include "hzpch.h"
#include "ScriptInstance.h"
#include "ScriptClass.h"
#include "ScriptEngine.h"

namespace Hazel
{
	ScriptInstance::ScriptInstance(Ref<ScriptClass> scriptClass, Entity entity)
		:_scriptClass(scriptClass)
	{
		// Base constructor
		_constructor = ScriptEngine::GetEntityClass()->GetMethod(".ctor", 1);

		_onCreateMethod = scriptClass->GetMethod("OnCreate");
		_onUpdateMethod = scriptClass->GetMethod("OnUpdate", 1);

		// Call Entity Constructor
		auto entityId = entity.GetUUID();
		void* param = &entityId;
		_instance = scriptClass->Instanciate(_constructor, &param);
	}

	void ScriptInstance::InvokeOnCreate()
	{
		_scriptClass->InvokeMethod(_instance, _onCreateMethod);
	}

	void ScriptInstance::InvokeOnUpdate(float timestep)
	{
		void* param = &timestep;
		_scriptClass->InvokeMethod(_instance, _onUpdateMethod, &param);
	}
}
