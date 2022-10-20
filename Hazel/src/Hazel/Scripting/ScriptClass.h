#pragma once

#include "ScriptField.h"

extern "C" // Forward declare of class from C
{
	typedef struct _MonoClass MonoClass;
	typedef struct _MonoObject MonoObject;
	typedef struct _MonoMethod MonoMethod;
	typedef struct _MonoClassField MonoClassField;
}

namespace Hazel
{
	class ScriptClass
	{
	public:
		ScriptClass() = default;
		ScriptClass(const std::string& classNamespace, const std::string& className, bool isCore = false);

		MonoObject* Instanciate(MonoMethod* constructor = nullptr, void** params = nullptr);
		MonoMethod* GetMethod(const std::string& name, int paramsCount = 0);
		MonoObject* InvokeMethod(MonoObject* instance, MonoMethod* monoMethod, void** params = nullptr);

		const std::string& GetFullName() const { return _classFullName; }
		const std::unordered_map<std::string, ScriptField>& GetFields() const { return _fields; }

	private:
		std::string _classNamespace;
		std::string _className;
		std::string _classFullName;

		std::unordered_map<std::string, ScriptField> _fields;

		MonoClass* _monoClass = nullptr;

		friend class ScriptEngine;
		friend class ScriptInstance;
	};
}
