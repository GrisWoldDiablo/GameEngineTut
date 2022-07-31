#pragma once

extern "C" // Forward declare of class from C
{
	typedef struct _MonoClass MonoClass;
	typedef struct _MonoObject MonoObject;
	typedef struct _MonoMethod MonoMethod;
}

namespace Hazel
{
	class ScriptClass
	{
	public:
		ScriptClass() = default;
		ScriptClass(const std::string& classNamespace, const std::string& className);

		MonoObject* Instanciate(MonoMethod* constructor = nullptr, void** params = nullptr);
		MonoMethod* GetMethod(const std::string& name, int paramsCount = 0);
		MonoObject* InvokeMethod(MonoObject* instance, MonoMethod* monoMethod, void** params = nullptr);

	private:
		std::string _classNamespace;
		std::string _className;
		MonoClass* _monoClass = nullptr;
	};
}
