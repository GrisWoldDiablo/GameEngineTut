#pragma once

extern "C" // Forward declare of class from C
{
	typedef struct _MonoClass MonoClass;
	typedef struct _MonoObject MonoObject;
	typedef struct _MonoMethod MonoMethod;
}

namespace Hazel
{
	class ScriptEngine
	{
	public:
		static void Init();
		static void Shutdown();

		static bool TryReload();

	private:
		static void InitMono();
		static void ShutdownMono();
		static bool TryLoadCSharpAssembly(const std::filesystem::path& filePath);

		static MonoObject* InstanciateClass(MonoClass* monoClass);

		static void DemoFunctionality();

		friend class ScriptClass;
	};

	class ScriptClass
	{
	public:
		ScriptClass() = default;
		ScriptClass(const std::string& classNamespace, const std::string& className);

		MonoObject* Instanciate();
		MonoMethod* GetMethod(const std::string& name, int parameterCount);
		MonoObject* InvokeMethod(MonoObject* instance, MonoMethod* monoMethod, void** params = nullptr);

	private:
		std::string _classNamespace;
		std::string _className;
		MonoClass* _monoClass = nullptr;
	};
}
