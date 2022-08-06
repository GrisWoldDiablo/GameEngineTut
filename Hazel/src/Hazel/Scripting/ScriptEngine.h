#pragma once

#include "Hazel/Scene/Entity.h"

extern "C" // Forward declare of class from C
{
	typedef struct _MonoClass MonoClass;
	typedef struct _MonoObject MonoObject;
	typedef struct _MonoMethod MonoMethod;
	typedef struct _MonoAssembly MonoAssembly;
	typedef struct _MonoImage MonoImage;
}

namespace Hazel
{
	class Scene;
	class ScriptClass;

	class ScriptEngine
	{
	public:
		static void Init();
		static void Shutdown();

		static void OnRuntimeStart(Scene* scene);
		static void OnRuntimeStop();
		static bool TryReload(bool shouldLog = true);

		static bool EntityClassExist(const std::string& fullClassName);
		static void OnCreateEntity(Entity entity);
		static void OnUpdateEntity(Entity entity, Timestep timestep);

		static Scene* GetSceneContext();
		static std::unordered_map<std::string, Ref<ScriptClass>> GetEntityClasses();

	private:
		static void InitMono();
		static void ShutdownMono();

		static bool TrySetupEngine();
		static bool TryLoadCSharpAssembly(const std::filesystem::path& filePath);
		static void LoadAssemblyClasses(MonoAssembly* assembly);

		static MonoObject* InstanciateClass(MonoClass* monoClass, MonoMethod* constructor = nullptr, void** params = nullptr);

		static MonoImage* GetCoreAssemblyImage();
		static Ref<ScriptClass> GetEntityClass();

		friend class ScriptGlue;
		friend class ScriptClass;
		friend class ScriptInstance;
	};
}
