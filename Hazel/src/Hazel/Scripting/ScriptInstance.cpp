#include "hzpch.h"
#include "ScriptInstance.h"
#include "ScriptClass.h"
#include "ScriptEngine.h"
#include "mono/metadata/object.h"

namespace Hazel
{
	ScriptInstance::ScriptInstance(Ref<ScriptClass> scriptClass, Entity entity)
		:_scriptClass(scriptClass)
	{
		// Base constructor
		_constructor = ScriptEngine::GetEntityClass()->GetMethod(".ctor", 1);

		_onCreateMethod = scriptClass->GetMethod("OnCreate");
		_onDestroyMethod = scriptClass->GetMethod("OnDestroy");
		_onUpdateMethod = scriptClass->GetMethod("OnUpdate", 1);

		// Call Entity Constructor
		auto entityId = entity.GetUUID();
		void* param = &entityId;
		_instance = scriptClass->Instanciate(_constructor, &param);
	}

	void ScriptInstance::InvokeOnCreate()
	{
		if (_onCreateMethod)
		{
			_scriptClass->InvokeMethod(_instance, _onCreateMethod);
		}
	}

	void ScriptInstance::InvokeOnDestroy()
	{
		if (_onDestroyMethod)
		{
			_scriptClass->InvokeMethod(_instance, _onDestroyMethod);
		}
	}

	void ScriptInstance::InvokeOnUpdate(float timestep)
	{
		if (_onUpdateMethod)
		{
			void* param = &timestep;
			_scriptClass->InvokeMethod(_instance, _onUpdateMethod, &param);
		}
	}

	bool ScriptInstance::TryGetFieldValueInternal(const std::string& name, void* data) const
	{
		const auto& fields = _scriptClass->GetFields();
		auto it = fields.find(name);
		if (it == fields.end())
		{
			return false;
		}

		const ScriptField& field = it->second;
		mono_field_get_value(_instance, field.MonoClassField, data);

		return true;
	}

	bool ScriptInstance::TrySetFieldValueInternal(const std::string& name, const void* data)
	{
		const auto& fields = _scriptClass->GetFields();
		auto it = fields.find(name);
		if (it == fields.end())
		{
			return false;
		}

		const ScriptField& field = it->second;
		mono_field_set_value(_instance, field.MonoClassField, const_cast<void*>(data));

		return true;
	}

	bool ScriptInstance::TryGetFieldStringValueInternal(const std::string& name, std::string& data) const
	{
		const auto& fields = _scriptClass->GetFields();
		auto it = fields.find(name);
		if (it == fields.end())
		{
			return false;
		}

		const ScriptField& field = it->second;
		auto* monoString = reinterpret_cast<MonoString*>(mono_field_get_value_object(mono_object_get_domain(_instance), field.MonoClassField, _instance));
		data = mono_string_to_utf8(monoString);

		return true;
	}

	bool ScriptInstance::TrySetFieldStringValueInternal(const std::string& name, const std::string& data)
	{
		const auto& fields = _scriptClass->GetFields();
		auto it = fields.find(name);
		if (it == fields.end())
		{
			return false;
		}

		const ScriptField& field = it->second;
		auto* monoString = mono_string_new(mono_object_get_domain(_instance), data.c_str());
		mono_field_set_value(_instance, field.MonoClassField, monoString);

		return true;
	}
}
