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
		ScriptField field;
		if (TryGetField(name, field))
		{
			mono_field_get_value(_instance, field.MonoClassField, data);
			return true;
		}

		return false;
	}

	bool ScriptInstance::TrySetFieldValueInternal(const std::string& name, const void* data)
	{
		ScriptField field;
		if (TryGetField(name, field))
		{
			mono_field_set_value(_instance, field.MonoClassField, const_cast<void*>(data));
			return true;
		}

		return false;
	}

	bool ScriptInstance::TryGetFieldStringValueInternal(const std::string& name, std::string& data) const
	{
		ScriptField field;
		if (TryGetField(name, field))
		{
			auto* monoString = reinterpret_cast<MonoString*>(mono_field_get_value_object(mono_object_get_domain(_instance), field.MonoClassField, _instance));
			data = mono_string_to_utf8(monoString);
			return true;
		}

		return false;
	}

	bool ScriptInstance::TrySetFieldStringValueInternal(const std::string& name, const std::string& data)
	{
		ScriptField field;
		if (TryGetField(name, field))
		{
			auto* monoString = mono_string_new(mono_object_get_domain(_instance), data.c_str());
			mono_field_set_value(_instance, field.MonoClassField, monoString);
			return true;
		}

		return false;
	}

	bool ScriptInstance::TryGetFieldEntityValueInternal(const std::string& name, Entity& data) const
	{
		ScriptField field;
		if (TryGetField(name, field))
		{
			if (auto* monoInstance = mono_field_get_value_object(mono_object_get_domain(_instance), field.MonoClassField, _instance))
			{
				const auto& scriptField = ScriptEngine::GetEntityClass()->GetFields().at("Id");
				uint64_t uuid;
				mono_field_get_value(monoInstance, scriptField.MonoClassField, &uuid);
				data = ScriptEngine::GetSceneContext()->GetEntityByUUID(uuid);

				return true;
			}
		}
		return false;
	}

	bool ScriptInstance::TrySetFieldEntityValueInternal(const std::string& name, const Entity& data)
	{
		ScriptField field;
		if (TryGetField(name, field))
		{
			// Clear the field.
			if (!data)
			{
				mono_field_set_value(_instance, field.MonoClassField, nullptr);
				return true;
			}

			// Assign the field to a entity that has an assigned ScriptInstance
			if (const auto& entityScriptInstance = ScriptEngine::GetEntityScriptInstance(data.GetUUID()))
			{
				mono_field_set_value(_instance, field.MonoClassField, entityScriptInstance->_instance);
				return true;
			}

			// Create a ScriptInstance and assign it to the entity.
			if (const auto& entityScriptInstance = ScriptEngine::OnCreateEntity(data))
			{
				mono_field_set_value(_instance, field.MonoClassField, entityScriptInstance->_instance);
				return true;
			}
		}

		return false;
	}

	bool ScriptInstance::TryGetField(const std::string& name, ScriptField& scriptField) const
	{
		const auto& fields = _scriptClass->GetFields();
		auto it = fields.find(name);
		if (it == fields.end())
		{
			return false;
		}

		scriptField = it->second;
		return true;
	}
}
