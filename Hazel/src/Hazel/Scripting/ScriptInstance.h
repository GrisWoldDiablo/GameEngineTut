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

	class ScriptInstance
	{
	public:
		ScriptInstance(Ref<ScriptClass> scriptClass, Entity entity);
		void InvokeOnCreate();
		void InvokeOnUpdate(float ts);

		Ref<ScriptClass> GetScriptClass() { return _scriptClass; }

		template<typename T>
		T GetFieldValue(const std::string& name)
		{
			if (TryGetFieldValueInternal(name, _sFieldValueBuffer))
			{
				return *reinterpret_cast<T*>(_sFieldValueBuffer);
			}
			//TODO error handling
			return T();
		}

		template<typename T>
		void SetFieldValue(const std::string& name, const T& data)
		{
			TrySetFieldValueInternal(name, &data);
			//TODO error handling
		}

	private:
		bool TryGetFieldValueInternal(const std::string& name, void* buffer);
		bool TrySetFieldValueInternal(const std::string& name, const void* buffer);

	private:
		Ref<ScriptClass> _scriptClass;

		MonoObject* _instance = nullptr;
		MonoMethod* _constructor = nullptr;
		MonoMethod* _onCreateMethod = nullptr;
		MonoMethod* _onUpdateMethod = nullptr;

		inline static char _sFieldValueBuffer[8];
	};
}
