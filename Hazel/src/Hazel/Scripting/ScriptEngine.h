#pragma once

#include "Hazel/Scene/Entity.h"

extern "C" // Forward declare of class from C
{
	typedef struct _MonoClass MonoClass;
	typedef struct _MonoObject MonoObject;
	typedef struct _MonoMethod MonoMethod;
	typedef struct _MonoAssembly MonoAssembly;
}

namespace Hazel
{
	class ScriptClass
	{
	public:
		ScriptClass() = default;
		ScriptClass(const std::string& classNamespace, const std::string& className);

		MonoObject* Instanciate(MonoMethod* constructor = nullptr, void** params = nullptr);
		MonoMethod* GetMethod(const std::string& name, int parameterCount = 0);
		MonoObject* InvokeMethod(MonoObject* instance, MonoMethod* monoMethod, void** params = nullptr);

	private:
		std::string _classNamespace;
		std::string _className;
		MonoClass* _monoClass = nullptr;
	};

	class ScriptInstance
	{
	public:
		ScriptInstance(Ref<ScriptClass> scriptClass, Entity entity);
		void InvokeOnCreate();
		void InvokeOnUpdate(float ts);

	private:
		Ref<ScriptClass> _scriptClass;

		MonoObject* _instance = nullptr;
		MonoMethod* _constructor = nullptr;
		MonoMethod* _onCreateMethod = nullptr;
		MonoMethod* _onUpdateMethod = nullptr;
	};

	class Scene;

	class ScriptEngine
	{
	public:
		static void Init();
		static void Shutdown();

		static void OnRuntimeStart(Scene* scene);
		static void OnRuntimeStop();
		static bool TryReload();

		static bool EntityClassExist(const std::string& fullClassName);
		static void OnCreateEntity(Entity entity);
		static void OnUpdateEntity(Entity entity, Timestep timestep);

		static Scene* GetSceneContext();
		static std::unordered_map<std::string, Ref<ScriptClass>> GetEntityClasses();

	private:
		static void InitMono();
		static void ShutdownMono();

		static bool TryLoadCSharpAssembly(const std::filesystem::path& filePath);

		static MonoObject* InstanciateClass(MonoClass* monoClass, MonoMethod* constructor = nullptr, void** params = nullptr);

		static void LoadAssemblyClasses(MonoAssembly* assembly);

		static void DemoFunctionality();

		friend class ScriptClass;
	};
}
