#pragma once
#include "Hazel/Scene/Entity.h"

extern "C" // Forward declare of class from C
{
	typedef struct _MonoObject MonoObject;
	typedef struct _MonoMethod MonoMethod;
}

namespace Hazel
{
	class ScriptClass;
	struct ScriptField;

	class ScriptInstance
	{
	public:
		ScriptInstance(Ref<ScriptClass> scriptClass, Entity entity);

		void InvokeOnCreate();
		void InvokeOnDestroy();
		void InvokeOnUpdate(float timestep);

		Ref<ScriptClass> GetScriptClass() const { return _scriptClass; }
		MonoObject* GetInstance() const { return _instance; }

		template<typename T>
		T GetFieldValue(const std::string& name) const
		{
			static_assert(sizeof(T) <= 16, "Type too large!");

			if (TryGetFieldValueInternal(name, _sFieldValueBuffer))
			{
				return *reinterpret_cast<const T*>(_sFieldValueBuffer);
			}
			//TODO error handling
			return T();
		}

		template<typename T>
		void SetFieldValue(const std::string& name, const T& data)
		{
			static_assert(sizeof(T) <= 16, "Type too large!");

			TrySetFieldValueInternal(name, &data);
			//TODO error handling
		}

		std::string GetFieldStringValue(const std::string& name) const
		{
			if (TryGetFieldStringValueInternal(name, _sFieldStringValueBuffer))
			{
				return _sFieldStringValueBuffer;
			}
			//TODO error handling
			return {};
		}

		void SetFieldStringValue(const std::string& name, const std::string& data)
		{
			TrySetFieldStringValueInternal(name, data);
			//TODO error handling
		}

		Entity GetFieldEntityValue(const std::string& name) const
		{
			if (TryGetFieldEntityValueInternal(name, _sFieldEntityValueBuffer))
			{
				return _sFieldEntityValueBuffer;
			}
			//TODO error handling
			return Entity();
		}

		void SetFieldEntityValue(const std::string& name, const Entity& data)
		{
			TrySetFieldEntityValueInternal(name, data);
			//TODO error handling
		}

	private:
		bool TryGetFieldValueInternal(const std::string& name, void* data) const;
		bool TrySetFieldValueInternal(const std::string& name, const void* data) const;

		bool TryGetFieldStringValueInternal(const std::string& name, std::string& data) const;
		bool TrySetFieldStringValueInternal(const std::string& name, const std::string& data) const;

		bool TryGetFieldEntityValueInternal(const std::string& name, Entity& data) const;
		bool TrySetFieldEntityValueInternal(const std::string& name, const Entity& data) const;

		bool TryGetField(const std::string& name, ScriptField& scriptField) const;

	private:
		Ref<ScriptClass> _scriptClass;

		MonoObject* _instance = nullptr;
		MonoMethod* _constructor = nullptr;
		MonoMethod* _onCreateMethod = nullptr;
		MonoMethod* _onDestroyMethod = nullptr;
		MonoMethod* _onUpdateMethod = nullptr;

		inline static uint8_t _sFieldValueBuffer[16];
		inline static std::string _sFieldStringValueBuffer;
		inline static Entity _sFieldEntityValueBuffer;

		friend class ScriptEngine;
	};
}
